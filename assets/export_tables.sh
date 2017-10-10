#!/bin/bash

# cd to script dir
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$script_dir"

# make temp dir
mkdir -p temp

# convert files to csv
libreoffice --headless --convert-to csv tables/{armor.ods,itemdrops.ods,monsters.ods,weapons.ods} --outdir temp/

# convert from csv to tsv
find temp/ -iname "*.csv" -exec sh -c 'f="{}";f="${f%.*}"; dos2unix "${f}.csv"; gawk -f scripts/csv2tsv.awk "${f}.csv" > "${f}.tsv"' \;

# move files
mv temp/*.tsv ../working/tables/

# remove temp files
rm temp/*
rm -fd temp
