** About libOSMScout **

Map version support:
===================

See libosmscout/include/osmscout/TypeConfig.h : FILE_FORMAT_VERSION

Map providers:
=============

See resources/map-providers.json

Sample request: $uri?fromVersion=22&toVersion=22&locale=us

[
  {"map":"europe\/albania","version":22,"directory":"europe\/albania-22-20210301-2256","timestamp":1614635815,"size":123392813,"name":"Albania"},
  {"map":"europe\/andorra","version":22,"directory":"europe\/andorra-22-20210301-2257","timestamp":1614635826,"size":4063842,"name":"Andorra"},
  {"map":"europe\/austria","version":22,"directory":"europe\/austria-22-20210301-2329","timestamp":1614637810,"size":869249731,"name":"Austria"},
  {"map":"russia","version":22,"directory":"russia-22-20210309","timestamp":1615249072,"size":"4188118798","name":"Russian Federation"},
  ...
  {"dir":"europe","name":"Europe"},
  {"dir":"north-america","name":"North America"},
  {"dir":"north-america\/us","name":"USA"},
  ...
  {"dir":"africa","name":"Africa"}
]

Voice providers:
===============

See resources/voice-providers.json

Sample request: $uri?locale=us

[
  {
    "lang": "American English",
    "gender": "female",
    "name": "Abby",
    "license": "CC-By-SA 3.0",
    "author": "Robert Marmorstein",
    "dir": "American English - Abby (female)",
    "description": "Abby is my four year old daughter. She did these mostly in a straight voice, but some of them are not quite how an adult would say things. She is excited about trying to be the voice of marble."
  },
  {
    "lang": "American English",
    "gender": "male",
    "name": "Bugsbane",
    "license": "CC-By-SA 3.0",
    "dir": "American English - Bugsbane (male)",
    "author": "Piers Duruz",
    "description": "35yo Australian / British Accent."
  },
  ...
]

