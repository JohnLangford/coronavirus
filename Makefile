all:	estimate_by_MSA.csv recent_mortality_by_county cumulative_cases_over_cumulative_deaths.png

COVID-19:
	git clone https://github.com/CSSEGISandData/COVID-19.git

cases_over_mortality: cases_over_mortality.cc
	g++ -Wall -g cases_over_mortality.cc -o cases_over_mortality

observations:	COVID-19 cases_over_mortality COVID-19/csse_covid_19_data/csse_covid_19_time_series/time_series_covid19_deaths_global.csv
	mkdir -p observations && cd observations && ../cases_over_mortality ../COVID-19/csse_covid_19_data/csse_covid_19_time_series/time_series_covid19_confirmed_global.csv ../COVID-19/csse_covid_19_data/csse_covid_19_time_series/time_series_covid19_deaths_global.csv

recent_mortality_by_county: recent_mortality_by_county.cc
	g++ -Wall -g recent_mortality_by_county.cc -o recent_mortality_by_county

estimate_by_MSA.csv: recent_mortality_by_county
	./recent_mortality_by_county COVID-19/csse_covid_19_data/csse_covid_19_time_series/time_series_covid19_deaths_US.csv cbsatocountycrosswalk.csv > temp.csv && head -n 1 temp.csv > $@ && tail -n +2 temp.csv | sort -n -r -t ',' -k 9 >> $@

clean:
	rm -rf COVID-19 observations cumulative_cases_over_cumulative_deaths.png cases_over_mortality *~

COVID-19-pull: FORCE
	cd COVID-19 && git pull --quiet && cd ..
FORCE: 

cumulative_cases_over_cumulative_deaths.png: COVID-19 COVID-19-pull cases_over_mortality observations plot_commands
	gnuplot plot_commands 

display: cumulative_cases_over_cumulative_deaths.png
	firefox cumulative_cases_over_cumulative_deaths.png
