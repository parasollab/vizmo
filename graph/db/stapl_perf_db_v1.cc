#include "db/stapl_perf_db_v1.h"

using namespace stapl;

void stapl_main(int argc, char **argv)
{
  stapl_perf_db_v1 db;

  string applications_name("testapp1");
  string
      applications_description
      ("applications self-test for class stapl_perf_db");
  string groups_name("testgroup1");
  string machines_name("testmachine1");
  string
      machines_description("machines self-test for class stapl_perf_db");
  string runs_description("runs self-test for class stapl_perf_db");
  string runs_timestamp("20040220123456");
  string statistics_value("1234");
  string statistics_info_name("teststat1");
  int statistics_info_fixed = 0;
  string
      statistics_info_description
      ("statistics_info self-test for class stapl_perf_db");
  string statistics_info_interpretation("INT");
  string users_csuser("testuser1");
  string users_name("users self-test for class stapl_perf_db");

  db.test_insert(applications_name, applications_description, groups_name,
                 machines_name, machines_description, runs_description,
                 runs_timestamp, statistics_value, statistics_info_name,
                 statistics_info_fixed, statistics_info_description,
                 statistics_info_interpretation, users_csuser, users_name);
  db.test_select(applications_name, applications_description, groups_name,
                 machines_name, machines_description, runs_description,
                 runs_timestamp, statistics_value, statistics_info_name,
                 statistics_info_fixed, statistics_info_description,
                 statistics_info_interpretation, users_csuser, users_name);
  db.test_delete(applications_name, applications_description, groups_name,
                 machines_name, machines_description, runs_description,
                 runs_timestamp, statistics_value, statistics_info_name,
                 statistics_info_fixed, statistics_info_description,
                 statistics_info_interpretation, users_csuser, users_name);
  db.test_select(applications_name, applications_description, groups_name,
                 machines_name, machines_description, runs_description,
                 runs_timestamp, statistics_value, statistics_info_name,
                 statistics_info_fixed, statistics_info_description,
                 statistics_info_interpretation, users_csuser, users_name);
}
