#!/bin/sh
echo -e "Publishing doxygen. \n"
set -e

if [ -n "$GH_TOKEN" ]; then
	cd docs/html
	git init
	git config --global user.email "travis@travis-ci.org"
	git config --global user.name "travis-ci"
	git add *
	git commit -m "Deployed to Github Pages"
	git push --force --quiet "https://${GH_TOKEN}@${GH_REF}" master:gh-pages
	echo -e "Published doxygen to gh-pages. \n"
fi


