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
#include "SqlTest.h"
#include "SqlColumnMatcher.h"
#include "SqlColumnAssignment.h"
#include "SqlStorable.h"
#include "SqlStatement.h"
#include "SqlTransaction.h"
//#include "PostgresConnector.h"
#include <thread>

using namespace ::metacpp;
using namespace ::metacpp::sql;

class City : public Object
{
public:
    int             id;
    String          name;

    META_INFO_DECLARE(City)
};

STRUCT_INFO_BEGIN(City)
    FIELD_INFO(City, id)
    FIELD_INFO(City, name, "Moscow")
STRUCT_INFO_END(City)

META_INFO(City)

DECLARE_STORABLE(City,
                 PRIMARY_KEY(COL(City::id)),
                 UNIQUE_INDEX(COL(City::id))
                 )

class Person : public Object
{
public:
    int                 id;
    String              name;
    Nullable<int>       age;
    Nullable<double>    cat_weight;
    int                 cityId;
    Nullable<DateTime>  birthday;

    META_INFO_DECLARE(Person)
};

STRUCT_INFO_BEGIN(Person)
    FIELD_INFO(Person, id)
    FIELD_INFO(Person, name)
    FIELD_INFO(Person, age)
    FIELD_INFO(Person, cat_weight)
    FIELD_INFO(Person, cityId)
    FIELD_INFO(Person, birthday)
STRUCT_INFO_END(Person)

META_INFO(Person)

DECLARE_STORABLE(Person,
                 PRIMARY_KEY(COL(Person::id)),
                 REFERENCES(COL(Person::cityId), COL(City::id)),
                 UNIQUE_INDEX(COL(Person::id)),
                 INDEX(COL(Person::cityId)),
                 CHECK(COL(Person::age), COL(Person::age) < 120)    // people do not live so much
                 )



TEST(StorableTest, testConstraints)
{
    ASSERT_EQ(Storable<Person>::numConstraints(), 5);
}

void SqlTest::SetUp()
{
    //m_conn = new connectors::postgres::PostgresConnector("dbname = postgres");
    m_conn = new connectors::sqlite::SqliteConnector("file:memdb?mode=memory&cache=shared");
    connectors::SqlConnectorBase::setDefaultConnector(m_conn);
    ASSERT_TRUE(m_conn->connect());
    prepareSchema();
}

void SqlTest::TearDown()
{
    connectors::SqlConnectorBase::setDefaultConnector(nullptr);
    delete m_conn;
}

TEST_F(SqlTest, transactionCommitTest)
{
    SqlTransaction transaction;
    ASSERT_NO_THROW(transaction.commit());
}

TEST_F(SqlTest, transactionRollbackTest)
{
    SqlTransaction transaction;
    ASSERT_NO_THROW(transaction.rollback());
}

TEST_F(SqlTest, transactionManualBegin)
{
    SqlTransaction transaction(SqlTransactionAutoCloseManual);
    ASSERT_NO_THROW(transaction.begin());
    ASSERT_ANY_THROW(transaction.begin());
    ASSERT_NO_THROW(transaction.commit());
    ASSERT_ANY_THROW(transaction.commit());
}

TEST_F(SqlTest, multipleTransactionsTest)
{
    const size_t numThreads = 10;
    std::thread threads[numThreads];
    for (size_t i = 0; i < numThreads; ++i)
    {
        threads[i] = std::thread([]()
            {
                SqlTransaction transaction;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            });
    }

    for (size_t i = 0; i < numThreads; ++i)
        if (threads[i].joinable())
            threads[i].join();
}


TEST_F(SqlTest, selectTest)
{
    prepareData();
    try
    {
        SqlTransaction transaction;
        Storable<Person> person;
        Storable<City> city;
        SqlResultSet resultSet = person.select().innerJoin<City>().where((COL(Person::age).isNull() ||
                (COL(Person::age) + 2.5  * COL(Person::cat_weight)) > 250) &&
                COL(Person::cityId) == COL(City::id) &&
                !lower(COL(Person::name)).like("invalid_%") &&
                COL(Person::birthday) > DateTime::fromISOString("2000-01-01 00:00:00"))
                .limit(10).orderAsc(COL(Person::name), COL(Person::age)).exec(transaction);

        StringArray persons;
        for (auto it : resultSet)
        {
            (void)it;
            persons.push_back(person.name);
        }

        person.remove().exec(transaction);

        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
    clearData();
}

TEST_F(SqlTest, updateTest)
{
    prepareData();
    try
    {
        prepareData();
        SqlTransaction transaction;
        Storable<Person> person;
        person.update().ref<City>().set(COL(Person::age) = null, COL(Person::cat_weight) = null)
                .where(COL(Person::cityId) == COL(City::id) && COL(City::name) == String("Moscow") &&
                       (COL(Person::age) == null || COL(Person::cat_weight) == null)).exec(transaction);
        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
    clearData();
}

TEST_F(SqlTest, insertTest)
{
    try
    {
        SqlTransaction transaction;
        Storable<City> city;
        auto resultSet = city.select().where(COL(City::name) == String("Moscow"))
                .exec(transaction);
        auto it = resultSet.begin();
        if (it != resultSet.end())
        {
            Storable<Person> person;
            person.init();
            person.cityId = city.id;
            person.name = "Pupkin";
            ASSERT_TRUE(person.insertOne(transaction));
            person.name = "Pupkin new";
            ASSERT_TRUE(person.updateOne(transaction));
            ASSERT_TRUE(person.removeOne(transaction));
        }

        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}

TEST_F(SqlTest, deleteTest)
{
    try
    {
        SqlTransaction transaction;
        Storable<Person> person;
        person.remove().ref<City>().where(COL(City::id) == COL(Person::cityId) &&
                                          COL(City::name) == String("Moscow") &&
                                          COL(Person::name).like("invalid_%")).exec(transaction);
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}

void SqlTest::prepareSchema()
{
    SqlTransaction transaction;
    Storable<City>::createSchema(transaction);
    Storable<Person>::createSchema(transaction);
    transaction.commit();
}

void SqlTest::prepareData()
{
    SqlTransaction transaction;
    Storable<City> city;
    city.init();
    city.name = "Moscow";
    city.insertOne(transaction);
    Storable<Person> person;
    person.init();
    person.name = "Pupkin";
    person.birthday = DateTime::fromISOString("2004-12-31 00:00:00");
    person.cityId = city.id;
    person.insertOne(transaction);
    transaction.commit();
}

void SqlTest::clearData()
{
    SqlTransaction transaction;
    Storable<City> city;
    Storable<Person> person;
    person.remove().exec(transaction);
    city.remove().exec(transaction);
    transaction.commit();
}
