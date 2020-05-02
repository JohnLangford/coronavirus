//To compile: g++ -Wall -g cases_over_mortality.cc -o cases_over_mortality 
#include<iostream>
#include<vector>
#include<strings.h>
#include<algorithm>
#include<string>
#include<fstream>

using namespace std;

struct case_series {
  string county;
  string state;
  size_t population;
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
	    case 1:
	    case 2:
	    case 3:
	    case 4:
	      break;
	    case 5:
	      s.county = parsed;
	      break;
	    case 6:
	      s.state = parsed;
	      break;
	    case 7:
	    case 8:
	    case 9:
	    case 10:
	      break;
	    case 11:	
	      //	      cout << "population string = " << parsed << endl;
	      s.population = atof(parsed.c_str());
	      // cout << "population = " << s.population << endl;
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

  return all_series;
}

int estimated_cases(case_series c)
{
  return 100*max(1,(int)(c.counts[c.counts.size()-1] - c.counts[c.counts.size()-8]));
}

bool compare_pop_over_fatality(case_series i1, case_series i2)
{
  return i1.population / (float) estimated_cases(i1) < i2.population / (float) estimated_cases(i2);
}

void print_case_series(case_series& c)
{
  if (c.county != "")
    cout << c.county << ", ";
  cout << c.state << " " << c.population;
  for (size_t i = 0; i < c.counts.size(); i++)
    cout << " " << c.counts[i];
  cout << endl;
}

int main(int argc, char* argv[])
{
  if (argc < 2)
    {
      cout << "usage: recent_mortality_by_county COVID-19/csse_covid_19_data/csse_covid_19_time_series/time_series_covid19_deaths_US.csv" << endl;
      exit (0);
    }

  vector<case_series> death_cumulatives = parse_series(argv[1]);

  sort(death_cumulatives.begin(), death_cumulatives.end(), compare_pop_over_fatality);
  
  cout << "county,state,population,last_week_deaths,population/deaths(min 1)" << endl;
  for (case_series c : death_cumulatives)
    {
      if (c.county.size() == 0 || c.county.find("Out_of_") != string::npos || c.county.find("Unassigned") != string::npos)
	continue;
      
      int last_week_deaths = max(0,(int)(c.counts[c.counts.size()-1] - c.counts[c.counts.size()-8]));
      cout << c.county << "," << c.state << ',' << c.population << ',' <<  last_week_deaths << "," << (float) c.population / (float) estimated_cases(c) << endl;
    }
}
