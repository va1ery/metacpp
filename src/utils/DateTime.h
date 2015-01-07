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
#ifndef METACPP_DATETIME_H
#define METACPP_DATETIME_H
#include <time.h>
#include "String.h"
#include "SharedDataPointer.h"

namespace metacpp {


class DateTimeData : public SharedDataBase
{
public:
    explicit DateTimeData(time_t stdTime = 0);
    explicit DateTimeData(const struct tm& tm);
    ~DateTimeData();

    bool operator==(const DateTimeData& rhs) const;
    bool operator!=(const DateTimeData& rhs) const;

    int year() const;
    int month() const;
    int day() const;
    int hours() const;
    int minutes() const;
    int seconds() const;

    time_t toStdTime() const;
    String toISOString() const;

    SharedDataBase *clone() const override;

    void fromISOString(const char *isoString);

private:
    struct tm m_tm;
};

class DateTime : protected SharedDataPointer<DateTimeData>
{
public:
    explicit DateTime(time_t stdTime);
    DateTime();
    ~DateTime();

    bool valid() const;

    bool operator==(const DateTime& rhs) const;
    bool operator!=(const DateTime& rhs) const;

    int year() const;
    int month() const;
    int day() const;
    int hours() const;
    int minutes() const;
    int seconds() const;

    time_t toStdTime() const;
    String toISOString() const;

    static DateTime fromISOString(const char *isoString);
    static DateTime now();
private:
    DateTimeData *getData() const;
};

} // namespace metacpp

#endif // METACPP_DATETIME_H
