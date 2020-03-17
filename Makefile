all:	COVID-19 cases_over_mortality observations plot_commands 
	cd COVID-19 && git pull && cd .. && gnuplot plot_commands && firefox cumulative_cases_over_cumulative_deaths.png

COVID-19:
	git clone https://github.com/CSSEGISandData/COVID-19.git

cases_over_mortality: cases_over_mortality.cc
	g++ -Wall -g cases_over_mortality.cc -o cases_over_mortality

observations:	COVID-19 cases_over_mortality COVID-19/csse_covid_19_data/csse_covid_19_time_series/time_series_19-covid-Confirmed.csv
	mkdir -p observations && cd observations && ../cases_over_mortality ../COVID-19/csse_covid_19_data/csse_covid_19_time_series/time_series_19-covid-Confirmed.csv ../COVID-19/csse_covid_19_data/csse_covid_19_time_series/time_series_19-covid-Deaths.csv

clean:
	rm -rf COVID-19 observations cumulative_cases_over_cumulative_deaths.png cases_over_mortality *~
