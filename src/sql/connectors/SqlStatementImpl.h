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
#ifndef SQLSTATEMENTIMPL_H
#define SQLSTATEMENTIMPL_H
#include <iostream>
#include <memory>
#include "String.h"
#include "Utils.h"
#include "SqlStatement.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{

/**
    \brief Base class for all statements
*/
class SqlStatementImpl
{
protected:
    SqlStatementImpl(SqlStatementType type, const String& queryText);
public:
    SqlStatementImpl(const SqlStatementImpl&)=delete;
    virtual ~SqlStatementImpl();

    /** \brief Get statement type */
    virtual SqlStatementType type() const;
    /** \brief Check if statement is prepared */
    virtual bool prepared() const;
    /** \brief Set prepared flag */
    virtual void setPrepared(bool val = true);
    /** \brief Check if we've done executing this statement (no more rows) */
    virtual bool done() const;
    /** \brief Set done flag */
    virtual void setDone(bool val = true);
    /** \brief Get sql query text */
    virtual const String& queryText() const;
protected:
    bool m_prepared;
    String m_queryText;
    SqlStatementType m_type;
    bool m_done;
};

} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif // SQLSTATEMENTIMPL_H

