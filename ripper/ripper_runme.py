#!/usr/bin/python3
import os
from urldown import *
from nonogram_ripper import *

# Config
out_dir = "cache"

# Make output directory
if not os.path.isdir(out_dir):
    os.makedirs(out_dir)

def download_puzzle(url):

    # Output filenames
    base = os.path.join(out_dir, url.split('/')[-1])
    json_file = base + '.json'
    png_file = base + '.png'

    # Skip if already downloaded
    if os.path.isfile(png_file):
        return

    # Pull puzzle
    print('Ripping ' + url + '...')
    ripper = nonogram_ripper(url)
    #ripper.export_json(json_file)
    ripper.render_png(png_file)
    
# Search through index of puzzles
next_page = 'http://www.nonograms.org/search?sort=0'
while next_page != None:
    cur_page = next_page
    next_page = None

    # Get page
    page = url_parse(cur_page)

    # Tick through all download links
    for link in page.find_all('a', href=True):

        # Download puzzle links
        if '/nonograms/i/' in link['href'] or '/nonograms2/i/' in link['href']:
            download_puzzle(link['href'])

        # Chain to next page if there
        if 'Next' in link.getText():
            next_page = link['href']
