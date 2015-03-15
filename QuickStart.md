# Introduction #

SVGMin reduces SVG size by throwing out redundant and useless information.

Many illustration programs can export the drawing to the SVG format. However, the SVG output is often sprinkled with a lot of extra stuff, mostly so that the file can be read back without a loss of information. This is however not good if the SVG output is the **final** file targeted for deployment, e.g. for a web site or as a scalable theme in a mobile platforms. The extra baggage in the contents which has **no effect** on the rendering should be removed to reduce both the download size and the (run-time) parsing time.

# Basic Usage #

SVGMin is a command line utility. You pass the input file and optionally give the output file. If no output file is given, the output will be dumped to the standard output.

An extremely simple example:

```
svgmin logo.svg logo-min.svg
```

You can pass some options to SVGMin to tweak the minification.

## Metadata ##

Metadata can be removed with the option `--remove-metadata` or kept with the option `--remove-metadata`. The default is to keep any metadata.

Before:

```
<svg><metadata><rdf:RDF><cc:Work><dc:format>image/svg+xml</dc:format>
</cc:Work></rdf:RDF></metadata>
<rect x="150" y="150" width="250" height="88"/></svg>
```

After:

```
<svg><rect x="150" y="150" width="250" height="88"/></svg>
```

## Style conversion ##

Style properties can be converted to XML attributes with the option `--style-to-xml=yes` or left as they are with option `--style-to-xml=no`. The default is to convert them to XML attributes.

SVG supports styles via CSS (cascading style sheet). Converting the style properties to XML attributes is preferable. The reasons for this is because extra style declaration will force the SVG loader to invoke its CSS parser, thereby slowing down the loading compared to the case where the attributes are available as normal XML ones.

Before:

```
<path style="fill:url(#g1);stroke:none" d="M 8.5,2.5 L 10,10 z" />
```

After:

```
<path fill="url(#g1)" stroke="none" d="M 8.5,2.5 L 10,10 z" />
```

## Editor data ##

The data specific to the illustration program can be removed with the option `--remove-editor-data` or left as it is with the option `--keep-editor-data`. The default is to
remove it.

Remove any editor data is useful if you would like to clean up the SVG from extra and irrelevant information produced by the illustration programs. However, this means that the resulting file is not always lossly editable by the illustration program.

Before:

```
<svg width="10" height="10" sodipodi:version="0.32"><defs>
<linearGradient inkscape:collect="always" x1="1" y1="1" x2="2" y2="2"/></defs>
<sodipodi:namedview  pagecolor="#ffffff" objecttolerance="10">
```

After:

```
<svg width="10" height="10"><defs>
<linearGradient x1="1" y1="1" x2="2" y2="2"/></defs>
```

## Automatic id ##

SVG elements can have an associated id, which is useful for the reference from other elements (e.g gradients), scripting purpose (e.g. web-based interface), or for partial rendering (e.g. theme).

An illustration program often generates the so-called _automatic id_, e.g. path1234 for a path element, where 1234 is a unique number. In most deployment cases, these id serve no purpose, which then can be removed by SVGMin.

By default, SVGMin removes all ids which start with `g, circle, path, polygon, polyline, rect, text`. To remove more ids, use the option `--remove-id=foo`, which removes all ids which start with foo. To keep certain ids, use the option `--keep-id=foo`.

Before:

```
<rect id="rect640" x="150" y="150" width="250" height="88"/>
<ellipse id="ellipse5197" cx="1300" cy="2700" rx="10" ry="2"/>
```

After:

```
<rect x="150" y="150" width="250" height="88"/>
<ellipse cx="1300" cy="2700" rx="10" ry="2"/>
```

Note: pattern-matching of id using regular expressions will be added in the future if there is enough demand for it.

# Caution #

If you use an illustration program, e.g. Inkscape, to work on the SVG file, SVGMin may destroys all the necessary information which is needed so that the file can be imported back. Thus, always keep the original version and treat the minified version as the exported version. Think of it like a document and its PDF version.

Thus, use SVGMin wisely.