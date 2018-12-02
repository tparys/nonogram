#!/usr/bin/python3
import urllib
from bs4 import BeautifulSoup

def url_raw(url):

    # Download data
    resp = urllib.request.urlopen(url)
    data = resp.read()

    return data

def url_parse(url):

    # Download data
    data = url_raw(url)

    # Parse HTML
    page = BeautifulSoup(data, 'html.parser')

    return page
