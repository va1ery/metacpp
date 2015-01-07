/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
#include "PostgresConnector.h"

namespace metacpp {
namespace sql {
namespace connectors {
namespace postgres {

PostgresConnector::PostgresConnector(const char *connectionString, int poolSize)
    : m_connectionString(connectionString), m_poolSize(poolSize), m_connected(false)
{
    if (m_poolSize <= 0)
        throw std::invalid_argument("Negative pool size");
    if (m_poolSize > 10)
        throw std::invalid_argument("Pool size is too large");
}

PostgresConnector::~PostgresConnector()
{
    if (m_connected)
        disconnect();
}

bool PostgresConnector::connect()
{

    if (m_connected)
    {
        std::cerr << "PostgresConnector::connect(): database connection seems to be already opened"
                  << std::endl;
        return true;
    }
    m_freeDbHandles.reserve(m_poolSize);
    for (int i = 0; i < m_poolSize; ++i)
    {
        PGconn *dbConn = PQconnectdb(m_connectionString.c_str());
        ConnStatusType status = PQstatus(dbConn);
        if (status != CONNECTION_OK)
        {
            std::cerr << "PQconnectdb(): failed to establish connection to database.";
            disconnect();
            return false;
        }
        m_freeDbHandles.push_back(dbConn);
    }
    return m_connected = true;
}

bool PostgresConnector::disconnect()
{
    if (!m_connected)
    {
        std::cerr << "PostgresConnector::disconnect(): database connection was not previously successfully created" << std::endl;
        return true;
    }

    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        if (m_transactions.size())
        {
            std::cerr << "PostgresConnector::disconnect(): there is still non-closed transaction connections left" << std::endl;
            return false;
        }
    }

    {
        std::lock_guard<std::mutex> _guard(m_poolMutex);
        assert(m_usedDbHandles.empty());
        for (size_t i = 0; i < m_freeDbHandles.size(); ++i)
            PQfinish(m_freeDbHandles[i]);
        m_freeDbHandles.clear();

    }
    return true;
}

SqlTransactionImpl *PostgresConnector::createTransaction()
{
    PGconn *dbConn = nullptr;
    {
        std::unique_lock<std::mutex> _guard(m_poolMutex);
        // check if there's already any free
        if (m_freeDbHandles.size())
        {
            dbConn = m_freeDbHandles.back();
            m_freeDbHandles.pop_back();
        }
        else
        {
            // predicate against spurious wakes
            m_dbHandleFreedEvent.wait(_guard, [this](){ return !m_freeDbHandles.empty(); });
            dbConn = m_freeDbHandles.back();
            m_freeDbHandles.pop_back();
        }
        m_usedDbHandles.push_back(dbConn);
    }

    PostgresTransactionImpl *result = new PostgresTransactionImpl(dbConn);
    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        m_transactions.push_back(result);
    }
    return result;
}

bool PostgresConnector::closeTransaction(SqlTransactionImpl *transaction)
{
    PGconn *dbConn = nullptr;
    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        PostgresTransactionImpl *postgresTransaction = reinterpret_cast<PostgresTransactionImpl *>(transaction);
        dbConn = postgresTransaction->dbConn();
        auto it = std::find(m_transactions.begin(), m_transactions.end(), postgresTransaction);
        if (it == m_transactions.end())
            return false;
        m_transactions.erase(it);
        delete transaction;
    }

    {
        std::lock_guard<std::mutex> _guard(m_poolMutex);
        auto it = std::find(m_usedDbHandles.begin(), m_usedDbHandles.end(), dbConn);
        if (it == m_usedDbHandles.end())
            throw std::runtime_error("PostgresConnector: No such used dbConn in connection pool");
        m_usedDbHandles.erase(it);
        m_freeDbHandles.push_back(dbConn);
        m_dbHandleFreedEvent.notify_all();
    }
    return true;
}

SqlSyntax PostgresConnector::sqlSyntax() const
{
    return SqlSyntaxPostgreSQL;
}

} // namespace postgres
} // namespace connectors
} // namespace sql
} // namespace metacpp

