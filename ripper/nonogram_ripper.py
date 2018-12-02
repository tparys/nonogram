#!/usr/bin/python3
import json
import png # pip3 install pypng
from urldown import *

class nonogram_ripper(object):

    # Constructor
    def __init__(self, url):

        # Superclass init
        object.__init__(self)

        # Download encoded puzzle from URL
        self.download_encoded_puzzle(url)
  
        # Decode dimensions
        self.decode_dimensions()
        
        # Decode color map
        self.decode_colors()

        # Decode puzzle
        self.decode_puzzle()

    # Download puzzle from url
    def download_encoded_puzzle(self, url):

        # Get Page
        page = url_parse(url)

        # Search for <script> tag with raw data
        for tags in page.find_all('script'):
            txt = tags.get_text()
            if 'var d=' in txt:
                txt = txt.replace('var d=','')
                txt = txt.replace(';','')
                self.encoded = eval(txt)
                return

        # Not there?
        raise Exception('Cannot find puzzle at URL')

    # Figure out dimensions
    def decode_dimensions(self):

        # Pull data
        self.ncols = self._f1(1)
        self.nrows = self._f1(2)

        # Progress
        print('size is %d rows x %d cols' % (self.nrows, self.ncols))
        
    # Figure out color map
    def decode_colors(self):

        # Pull data
        self.ncolors = self._f1(3)

        # Set up colors, with a transparent bg
        self.colors = []
        self.add_color(0, 255, 255, 255, 0)

        # Read each color
        for i in range(0, self.ncolors):
            j = i + 5
            red   = self.encoded[j][0] - self.encoded[4][1]       # Aa
            green = self.encoded[j][1] - self.encoded[4][0]
            blue  = self.encoded[j][2] - self.encoded[4][3]
            fg    = self.encoded[j][3] - self.encoded[4][2] - red # S
            self.add_color(255, red, green, blue, fg)

    # Decode puzzle
    def decode_puzzle(self):

        # Create an empty grid to fill in
        self.grid = []
        for r in range(0, self.nrows):
            self.grid.append([])
            for c in range(0, self.ncols):
                self.grid[r].append(0)          # bg "color"

        # This section is obfuscated pretty well ...
        V = self.ncolors + 5 # xa + 5
        Ba = self._f2(V)
        Ca = self.encoded[V + 1]
        for Q in range(V + 2, V + 2 + Ba):
            for U in range(self.encoded[Q][0] - Ca[0] - 1, self.encoded[Q][0] - Ca[0] + self.encoded[Q][1] - Ca[1] - 1):
                self.grid[self.encoded[Q][3] - Ca[3] - 1][U] = self.encoded[Q][2] - Ca[2]

    # Render to console
    def render_console(self):
        for r in range(0, self.nrows):
            work = ''
            for c in range(0, self.ncols):
                work += self.color_text(' ', self.grid[r][c])
            print(work)

    # Export to JSON
    def export_json(self, filename):

        # Empty structure to export
        puz_data = {}

        # Dimensions
        puz_data['ncols'] = self.ncols
        puz_data['nrows'] = self.nrows

        # Colors
        tmp = []
        for color in self.colors[1:]:
            tmp.append('%02x%02x%02x' % (color['red'], color['blue'], color['green']))
        puz_data['colors'] = tmp

        # Reference grid data
        puz_data['puzzle'] = self.grid

        # Out it goes
        text = json.dumps(puz_data) # , indent=2, sort_keys=True)
        print(text, file=open(filename, 'w'))

    # Render to PNG file
    def render_png(self, filename):

        # Build a RGBA palette for PNG library
        palette = []
        for color in self.colors:
            palette.append((color['red'], color['green'], color['blue'], color['alpha']))

        # Set up PNG writer
        w = png.Writer(len(self.grid[0]), len(self.grid), palette=palette, bitdepth=8, compression=5)

        # Open output file and stream data
        f = open(filename, 'wb')
        w.write(f, self.grid)

    # Add next color code
    def add_color(self, alpha, red, green, blue, fg):

        # Add color to palette
        cidx = len(self.colors)
        next_color = {'alpha' : alpha, 'red' : red, 'green' : green, 'blue' : blue, 'fg' : fg}
        self.colors.append(next_color)

        # Console output
        colorcode = '%02x%02x%02x%02x' % (alpha, red, green, blue)
        colorized = self.color_text(colorcode, cidx)
        print('color[%d] is %s' % (cidx, colorized))

    # Make some text into color
    def color_text(self, text, cidx):

        # Pull appropriate color parts
        f = self.colors[cidx]['fg']
        r = self.colors[cidx]['red']
        g = self.colors[cidx]['green']
        b = self.colors[cidx]['blue']

        # Foreground color code
        if f == 0:
            fg = '30'
        else:
            fg = '37'

        # Finish up whole sequence
        return (self.make_escape('%s;48;2;%d;%d;%d' % (fg, r, g, b)) +
            text + self.make_escape(''))

    # Gin up a ANSI escape sequence
    def make_escape(self, data):
        return '%c[%sm' % (27, data)

    # Misc helper functions, lots of modulo arithmetic
    def _f0(self, i, j):
        return self.encoded[i][j] % self.encoded[i][3]

    # used in # columns, rows, and colors
    def _f1(self, i):
        return self._f0(i, 0) + self._f0(i, 1) - self._f0(i,2)

    # used in game #, and 
    def _f2(self, i):
        return (self._f0(i, 0) * self._f0(i, 0)) + 2 * (self._f0(i, 1)) + self._f0(i, 2)

