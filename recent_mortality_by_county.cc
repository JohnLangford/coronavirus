//To compile: g++ -Wall -g cases_over_mortality.cc -o cases_over_mortality 
#include<iostream>
#include<vector>
#include<strings.h>
#include<algorithm>
#include<unordered_map>
#include<string>
#include<fstream>

#define DAYS 7

using namespace std;

struct case_series {
  string county;
  string state;
  size_t population;
  vector<float> counts;
};

unordered_map<string,string> state_abbreviations;

void upper_case(string& input)
{
  for(unsigned int i=0;i<input.length();i++)
    input[i] = toupper(input[i]);
}
   
void add_abbrev(string state, string abbrev)
{
  upper_case(state);
  state_abbreviations.emplace(state,abbrev);
}

void intialize_abbreviations()
{
  add_abbrev("Alabama","AL");
  add_abbrev("Alaska","AK");
  add_abbrev("Arizona","AZ");
  add_abbrev("Arkansas","AR");
  add_abbrev("California","CA");
  add_abbrev("Colorado","CO");
  add_abbrev("Connecticut","CT");
  add_abbrev("Delaware","DE");
  add_abbrev("Florida","FL");
  add_abbrev("Georgia","GA");
  add_abbrev("Hawaii","HI");
  add_abbrev("Idaho","ID");
  add_abbrev("Illinois","IL");
  add_abbrev("Indiana","IN");
  add_abbrev("Iowa","IA");
  add_abbrev("Kansas","KS");
  add_abbrev("Kentucky","KY");
  add_abbrev("Louisiana","LA");
  add_abbrev("Maine","ME");
  add_abbrev("Maryland","MD");
  add_abbrev("Massachusetts","MA");
  add_abbrev("Michigan","MI");
  add_abbrev("Minnesota","MN");
  add_abbrev("Mississippi","MS");
  add_abbrev("Missouri","MO");
  add_abbrev("Montana","MT");
  add_abbrev("Nebraska","NE");
  add_abbrev("Nevada","NV");
  add_abbrev("New_Hampshire","NH");
  add_abbrev("New_Jersey","NJ");
  add_abbrev("New_Mexico","NM");
  add_abbrev("New_York","NY");
  add_abbrev("North_Carolina","NC");
  add_abbrev("North_Dakota","ND");
  add_abbrev("Ohio","OH");
  add_abbrev("Oklahoma","OK");
  add_abbrev("Oregon","OR");
  add_abbrev("Pennsylvania","PA");
  add_abbrev("Rhode_Island","RI");
  add_abbrev("South_Carolina","SC");
  add_abbrev("South_Dakota","SD");
  add_abbrev("Tennessee","TN");
  add_abbrev("Texas","TX");
  add_abbrev("Utah","UT");
  add_abbrev("Vermont","VT");
  add_abbrev("Virginia","VA");
  add_abbrev("Washington","WA");
  add_abbrev("West_Virginia","WV");
  add_abbrev("Wisconsin","WI");
  add_abbrev("Wyoming","WY");
  add_abbrev("District_of_Columbia","DC");
  add_abbrev("Marshall_Islands","MH");
}

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
      after_last_comma++;
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
      if (next_comma > after_last_comma && *(next_comma-1)== '_')
	*(next_comma-1)='\0';
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
	  upper_case(parsed);
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
	      s.state = state_abbreviations[parsed];
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
  return 100*max(1,(int)(c.counts[c.counts.size()-1] - c.counts[c.counts.size()-1-DAYS]));
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

string county_state(string& county, string& state)
{
  return county + "," + state;
}

unordered_map<string,string> parse_county_msa(char* file)
{
  FILE* file_pointer = fopen(file, "r");
  if (file_pointer == nullptr)
    {
      perror("fopen");
      exit(EXIT_FAILURE);
    }

  unordered_map<string,string> county_msas;
  char* line = nullptr;
  size_t len = 0;
  ssize_t nread = getline(&line, &len, file_pointer);
  while ((nread = getline(&line, &len, file_pointer)) != -1)
    {
      string county;
      string state;
      string msa;
      char* after_last_comma = line;
      int item=0;
      while (after_last_comma != nullptr)
	{
	  string parsed = parse_next_entry(after_last_comma);
	  switch (item)
	    {
	    case 0:
	      county=parsed;
	      break;
	    case 1:
	      state=parsed;
	      break;
	    case 2:
	    case 3:
	    case 4:
	    case 5:
	      break;
	    case 6:
	      msa = parsed;
	      break;
	    }
	  item++;
	}
      county_msas[county_state(county,state)]= msa;
    }
  fclose(file_pointer);
  free(line);

  return county_msas;
}

struct msa_state {
  string msa;
  size_t population;
  size_t mortality;
};

int main(int argc, char* argv[])
{
  if (argc < 2)
    {
      cout << "usage: recent_mortality_by_county COVID-19/csse_covid_19_data/csse_covid_19_time_series/time_series_covid19_deaths_US.csv cbsatocountycrosswalk.csv" << endl;
      exit (0);
    }

  intialize_abbreviations();
  
  vector<case_series> death_cumulatives = parse_series(argv[1]);

  unordered_map<string,string> county_msas = parse_county_msa(argv[2]);
  
  unordered_map<string,msa_state> msa_cumulatives;
  size_t missing_deaths =0;
  size_t missing_population=0;
  for (case_series c: death_cumulatives)//need to map counties to MSAs
    {
      string msa;
      if (county_msas.find(county_state(c.county,c.state)) == county_msas.end())
	{
	  missing_deaths+= max(0,(int)(c.counts[c.counts.size()-1] - c.counts[c.counts.size()-1-DAYS]));
	  missing_population+=c.population;
	  if (c.county.find("OUT_OF_")==string::npos && c.county.find("UNASSIGNED")==string::npos && c.county.length() != 0)
	    cerr << "can not find msa for " << county_state(c.county,c.state) << endl;
	  continue;
	}
      msa = county_msas[county_state(c.county,c.state)];
      
      if (msa_cumulatives.find(msa)==msa_cumulatives.end())
	{
	  msa_state s = {msa,0,0};
	  msa_cumulatives.emplace(msa,s);
	}
      msa_cumulatives[msa].population += c.population;
      msa_cumulatives[msa].mortality += max(0,(int)(c.counts[c.counts.size()-1] - c.counts[c.counts.size()-1-DAYS]));
    }
  cerr << "deaths not assigned to MSA = " << missing_deaths << endl;
  cerr << "population not assigned to MSA = " << missing_population << endl;
  
  cout << "MSA,population,last_week_deaths,estimated_active_cases(min 50),estimated_cases_per_day,fast_tests_per_day,initial_active_tracers,tests_to_clear,population/estimated_cases" << endl;
  for (auto c : msa_cumulatives)
    {
      size_t estimated_cases = max((size_t)50,c.second.mortality*100);
      size_t estimated_cases_per_day = estimated_cases / DAYS;
      size_t fast_tests_per_day = estimated_cases_per_day * 20;
      size_t initial_active_tracers = estimated_cases_per_day*5;
      size_t tests_to_clear = estimated_cases*100;
	
      cout << c.first
	   << ',' << c.second.population
	   << ',' <<  c.second.mortality
	   << ',' << estimated_cases
	   << ',' << estimated_cases_per_day
	   << ',' << fast_tests_per_day
	   << ',' << initial_active_tracers
	   << ',' << tests_to_clear
	   << ',' << (float) c.second.population / (float) estimated_cases << endl;
    }
}
