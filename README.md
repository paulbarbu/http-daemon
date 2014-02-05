HTTP Daemon
===========
Simple HTTP daemon created for learning purposes using Qt and C++.

By no means a production ready or a fully RFC-compilant daemon.

Installation
============
The daemon must be linked to the http-daemon-components library
(provided separately). In order to do this the paths in the project file must
be changed to point to the header files of the http-daemon-components and to
the library provided by the same project.

The qmake variables that need to be changed are: `LIBS`, `INCLUDEPATH`,
`DEPENDPATH` and `PRE_TARGETDEPS`.

License
=======
Copyright 2013-2014 Barbu Paul - Gheorghe

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
