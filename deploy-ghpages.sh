#!/bin/sh
echo "Publishing doxygen. \n"
if [ -n "$GH_TOKEN" ]; then
	cd docs/html
	git init
	git config --global user.email "travis@travis-ci.org"
	git config --global user.name "travis-ci"
	git add *
	git commit -m "Deployed to Github Pages"
	git push --force --quiet "https://${GH_TOKEN}@github.com/brtos/brtos.git" master:gh-pages
	echo "Published doxygen to gh-pages. \n"
fi
echo "Published doxygen failed. \n"