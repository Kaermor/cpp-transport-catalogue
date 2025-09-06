# Transport catalogue

All about bus routes in big city. A program that stores and processes information about bus stops and buses.

- Accepts JSON data as input and returns an SVG file that visualizes stops and routes.
- Finds the shortest route between stops.
- Finds information about buses passing through the selected stop.
- Finds information about route (length, stops, curvature)
- Implemented a JSON constructor that allows finding an incorrect sequence of methods at compile.

## Input data format
- Input data is received by the program from stdin in the JSON object format, which has the following structure at the top level:
``` cpp
{
"base_requests": [ ... ],    \\ array with description of bus routes and stops
"render_settings": { ... },  \\ dictionary of settings for rendering the image
"routing_settings": { ... }, \\ dictionary of settings for constructing routes
"stat_requests": [ ... ]     \\ array with requests to the transport catalogue
}
```

- Transport catalogue database filling request
``` cpp
{
"type": "Bus",      \\ adding a bus to the catalog
"name": "...",      \\ bus number
"stops": [...],     \\ names of stops through which the bus passes
"is_roundtrip": ... \\ route type: true - circular pathway, false - direct
},
{
"type": "Stop",         \\ adding a stop to the catalog
"name": "...",          \\ name of the stop
"latitude": ...,        \\ coordinate by width
"longitude": ...,       \\ coordinate by longitude
"road_distances": {...} \\ known distance to other stops
},
```

- Map visualization setting set request
``` cpp
"width": ...,                  \\ specifies the width in pixels (real number in the range from 0 to 100000)
"height": ...,                 \\ specifies the height in pixels (real number in the range from 0 to 100000.)
"padding": ...,                \\ indentation of the edges of the map from the boundaries of the SVG document (real number not less than 0 and less than min(width, height)/2)
"stop_radius": ...,            \\ radius of the circles that denote the stops (real number in the range from 0 to 100000)
"line_width": ...,             \\ the thickness of the lines used to draw bus routes (a real number in the range from 0 to 100000)
"bus_label_font_size": ...,    \\ the size of the text used to write the names of bus routes (an integer in the range from 0 to 100000)
"bus_label_offset": [..., ...],\\ the offset of the label with the name of the route relative to the coordinates of the final stop on the map (elements of type double in the range from –100000 to 100000); sets the values ​​of the dx and dy properties of the SVG <text> element
"stop_label_font_size": ...,   \\ the size of the text used to display stop names (an integer in the range from 0 to 100000)
"stop_label_offset": [...],    \\ the offset of the stop name relative to its coordinates on the map (elements of type double in the range from –100000 to 100000); sets the values ​​of the dx and dy properties of the SVG <text> element
"underlayer_color": [...],     \\ names of stops and routes background color
"underlayer_width": ...,       \\ names of stops and routes background thickness (a real number in the range from 0 to 100000); sets the value of the stroke-width attribute of the <text> element
"color_palette": [...]         \\ color palette (non-empty array)
```

The color can be specified:
- as a string, such as `"red"` or `"black"`;
- as an array of three integers in the range `[0, 255]`. They define the `r`, `g`, and `b` components of the color in the `svg::Rgb` format. The color `[255, 16, 12]` should be output as `rgb(255, 16, 12)`;
- as an array of four elements: three integers in the range `[0, 255]` and one real number in the range `[0.0, 1.0]`. They define the `red`, `green`, `blue`, and `opacity` components of the color in the `svg::Rgba` format. A color specified as `[255, 200, 23, 0.85]` will be output as `rgba(255, 200, 23, 0.85)`.

- Route building settings set request
``` cpp
"bus_wait_time": ..., \\ bus waiting time at a stop, in minutes
"bus_velocity": ...   \\ bus speed, in km/h
```

- Requests to the transport catalogue
``` cpp
{ "id": ..., "type": "Stop", "name": "..." },              \\ request to display stop information
{ "id": ..., "type": "Bus", "name": "..." },               \\ request to display route information
{ "id": ..., "type": "Map" },                              \\ request to display SVG map
{ "id": ..., "type": "Route", "from": "...", "to": "..." } \\ request to display information about the fastest route
```

## Output format
- Request to display stop information
``` cpp
{
"buses": [
...               \\ route numbers that pass through this stop, in quotes, separated by commas (if there are no stops - "")
],
"request_id" :... \\ request id
}
```

- Route information output request
``` cpp
{
"request_id": ...,       \\ request id
"curvature": ...,        \\ ratio of the actual route length to the geographic distance (real number greater than one*)
"route_length": ...,     \\ route length in meters
"stop_count": ...,       \\ total number of stops on the route
"unique_stop_count": ... \\ number of unique stops on the route
}
```

- Request to output a map in SVG format
``` cpp
{
"map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">...\n</svg>",
"request_id": ...
}
```

- Shortest/fastest route output request
``` cpp
{
"request_id": ..., \\ request id
"total_time": ..., \\ total time
"items": [         \\ route elements
{
"stop_name": "...", \\ stop name
"time": ...,        \\ waiting time at stop
"type": "Wait"      \\ waiting at stop
},
{
"bus": "...",      \\ travel by bus
"span_count": ..., \\ number of stops passed
"time": ...,       \\ travel time
"type": "Bus"      \\ bus number
}, ...
]
}
```

## Deployment and requirements
C++17. No additional requirements.
