//To compile: g++ -Wall -g cases_over_mortality.cc -o cases_over_mortality 
#include<iostream>
#include<vector>
#include<strings.h>
#include<algorithm>
#include<string>
#include<fstream>

using namespace std;

struct case_series {
  string province;
  string country;
  vector<float> counts;
};

void clean_entry(char *start, char* end, char val)
{
  char* loc = index(start, val);
  if (loc != nullptr && loc < end)
    {
      *loc = '_';
      clean_entry(start, end, val);
    }
}

string parse_next_entry(char*& after_last_comma)
{
  string ret;
  char* parse_start = after_last_comma;
  if (*parse_start == '"')//eat any quoted characters which may include comma
    {
      *parse_start = '_';
      char* next_quote = index(parse_start+1, '"');
      clean_entry(parse_start, next_quote, ',');
      clean_entry(parse_start, next_quote, ' ');
      *next_quote = '_';
      parse_start = next_quote;
    }
  char* next_comma = index(parse_start, ',');
  if (next_comma != nullptr)
    {
      *next_comma = '\0';
      clean_entry(parse_start, next_comma, ' ');
      clean_entry(parse_start, next_comma, '*');
      ret = after_last_comma;
      after_last_comma = next_comma+1;
    }
  else
    {
      ret = after_last_comma;
      after_last_comma = nullptr;
    }
  return ret;
}

bool compare_country(case_series i1, case_series i2)
{
  return i1.country < i2.country;
}

vector<case_series> daily(vector<case_series>& cs)
{
  vector<case_series> ret;
  for (case_series c: cs)
    {
      case_series daily = c;
      for (size_t i = 1; i < c.counts.size(); i++)
	daily.counts[i]=c.counts[i]-c.counts[i-1];
      for (size_t i = 0; i < c.counts.size()-1; i++)
	if (daily.counts[i]==0)
	  {
	    daily.counts[i]=daily.counts[i+1]/2;
	    daily.counts[i+1]=daily.counts[i+1]/2;
	  }
      ret.push_back(daily);
    }
  return ret;
}

vector<case_series> parse_series(char* file)
{
  FILE* file_pointer = fopen(file, "r");
  if (file_pointer == nullptr)
    {
      perror("fopen");
      exit(EXIT_FAILURE);
    }

  vector<case_series> all_series;
  char* line = nullptr;
  size_t len = 0;
  ssize_t nread = getline(&line, &len, file_pointer);
  while ((nread = getline(&line, &len, file_pointer)) != -1)
    {
      case_series s;
      char* after_last_comma = line;
      int item=0;
      while (after_last_comma != nullptr)
	{
	  string parsed = parse_next_entry(after_last_comma);
	  switch (item)
	    {
	    case 0:
	      s.province = parsed;
	      break;
	    case 1:
	      s.country = parsed;
	      break;
	    case 2:
	      break;
	    case 3:
	      break;
	    default:
	      s.counts.push_back(atof(parsed.c_str()));
	    }
	  item++;
	}
      all_series.push_back(s);
    }
  fclose(file_pointer);
  free(line);

  //add countries without provinces.
  sort(all_series.begin(), all_series.end(), compare_country);
  vector<case_series> province_free_series;
  for (case_series& c: all_series)
    {
      if (c.province != "")
	{
	  if (province_free_series.size() > 0 && c.country == province_free_series.back().country)
	    for (size_t i = 0; i < c.counts.size(); i++)
	      province_free_series.back().counts[i] += c.counts[i];
	  else
	    {
	      province_free_series.push_back(c);
	      province_free_series.back().province="";
	    }
	}
    }

  //Get China without Hubei
  case_series china_except_hubei;
  china_except_hubei.country = "China";
  china_except_hubei.province = "Except_Hubei";
  china_except_hubei.counts.resize(all_series.front().counts.size(), 0);
  for (case_series& c: all_series)
    if (c.country == "China" && c.province != "Hubei")
      {
	if (china_except_hubei.counts.size() != c.counts.size())
	  {
	    cout << "mismatched counts!" << endl;
	    continue;
	  }
	for (size_t i = 0; i < c.counts.size(); i++)
	  china_except_hubei.counts[i] += c.counts[i];
      }
  all_series.push_back(china_except_hubei);

  for (case_series& c: province_free_series)
    all_series.push_back(c);

  return all_series;
}

void print_case_series(case_series& c)
{
  if (c.province != "")
    cout << c.province << ", ";
  cout << c.country;
  for (size_t i = 0; i < c.counts.size(); i++)
    cout << " " << c.counts[i];
  cout << endl;
}

int main(int argc, char* argv[])
{
  if (argc < 2)
    {
      cout << "usage: read_cases <confirmed> <deaths>" << endl;
      exit (0);
    }

  vector<case_series> confirmed_cumulatives = parse_series(argv[1]);
  vector<case_series> death_cumulatives = parse_series(argv[2]);

  vector<case_series> daily_confirmed = daily(confirmed_cumulatives);

  vector<case_series> cumulative_confirmed_over_deaths;
  vector<case_series> daily_confirmed_after_death;
  for (size_t i = 0; i < confirmed_cumulatives.size(); i++)
    {
      case_series c,d;
      c.country = confirmed_cumulatives[i].country;
      c.province = confirmed_cumulatives[i].province;
      d.country = c.province;
      d.province = c.province;
      for (size_t j = 0; j < confirmed_cumulatives[i].counts.size(); j++)
	if (death_cumulatives[i].counts[j] != 0)
	  {
	    c.counts.push_back(confirmed_cumulatives[i].counts[j]/death_cumulatives[i].counts[j]);
	    d.counts.push_back(daily_confirmed[i].counts[j]);
	  }
      if (c.counts.size() > 0)
	{
	  cumulative_confirmed_over_deaths.push_back(c);
	  daily_confirmed_after_death.push_back(d);
	}
    }
  
  for (size_t i =0; i < cumulative_confirmed_over_deaths.size(); i++)
    {
      string file_name = cumulative_confirmed_over_deaths[i].country;
      if (cumulative_confirmed_over_deaths[i].province != "")
	{
	  file_name += "_";
	  file_name += cumulative_confirmed_over_deaths[i].province;
	}
      
      fstream fs (file_name.c_str(), std::fstream::out);
      for (size_t j = 0; j < cumulative_confirmed_over_deaths[i].counts.size(); j++)
	{
	  fs << cumulative_confirmed_over_deaths[i].counts[j] << " " << daily_confirmed_after_death[i].counts[j];
	  if (j+1 < cumulative_confirmed_over_deaths[i].counts.size())
	    fs << " " << cumulative_confirmed_over_deaths[i].counts[j+1] - cumulative_confirmed_over_deaths[i].counts[j]  << " " << daily_confirmed_after_death[i].counts[j+1] - daily_confirmed_after_death[i].counts[j];
	  else
	    fs << " 0 0";
	  fs << " " << j;
	  fs << endl;
	}
      fs.close();
    }
}
