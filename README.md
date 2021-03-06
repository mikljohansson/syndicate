Synd is a software framework written in PHP, used primarily for
building dynamic websites.

Synd is tested on Linux/Apache2 with PHP 4.3+ or 5.0. Database
drivers are supplied for Oracle, MySQL and PostgreSQL. The cluster
driver provides load-balancing and transparent failover.
  
## Some features include:
  
   - Modular/Hooks based API
   - Object->RDBMS (ActiveRecord) mapping layer
   - Template system with override directories for skinning
     support. Node and type templates are Model-View-Controller 
     views with which an object can be rendered and are inherited
     allowing for object oriented techniques like polymorphism.
     Detection of cellphones and PDA's with template override 
     directories for multi-target rendering.
   - Integrated search engine supporting several index backends
     Term indexing uses a regular database backend and the BM25 
       weighting algorithm. The query parser handles all types of 
       boolean queries.
     MySQL fulltext indexes are both fast and allows for rating 
       matches. Boolean queries are supported.
     Wrappers for Xapian is provided. Xapian is an Open Source 
       Probabilistic Information Retrieval library written in C++
       and suitable for large collections in the gigabyte(s) range.
   - Transaction support. Multi-table-spanning nodes are always synced 
     atomically. Database independant methods for automatically or 
     manually performing rollbacks on errors are provided.
   - Clustered storage system with replication, failover and self 
     healing features. Uploaded files are replicated across a user
     defined number of storage nodes.
   - User-defined PHP extensions compiled and loaded on demand
   - Offline task API for performing time-consuming operations such
     as downsampling mp3's and movies for streaming to users
   - Apache mod_rewrite support to enable clean, search engine 
     friendly uri's
   - Image resizing and caching of resized images
   - HtmlFilter and HtmlTidy support that strips evil JavaScript and 
     fixes broken HTML in user-submitted data
   - Extensive unit-tests using PEAR::PHPUnit
   - Autogenerated documentation using PEAR::PhpDocumentor
   - Pingback support via mod_xmlrpc, trackback receiving support
   - JavaScript XML-RPC implementation
   - Oracle Calendar extension and OO wrapper

  Synd is distributed under the GNU General Public License - see the
  accompanying LICENSE file for more details. Commercial non GPL 
  licenses are available for users who prefer not to be restricted by
  the terms of the GPL.

## Modules
  
  Modules can be turned on/off via the configuration file, lazy
  loading is supported. Some featured modules:
  
   - Issue and project tracking module; featuring among other thigs
     subprojects, keywords, subissues (1st/2nd line support), email 
     integration, time logging and service level reporting.
   - Inventory module integrated with the issue tracker to handle
     repair and replacement issues. A Windows (C# .NET service) and
     Unix (shell script) compatible software agent, running on
     client computers, automates inventory deployment and maintenance.
   - Multi-tier cache module with support for memory, disk and
     distributed cache schemes. Objects and template/script output 
     can be cached with invalidation callbacks. Caching can be load 
     sensitive in order to handle spikes gracefully.
   - XML-RPC and SOAP support allowing remote method invocation on 
     all framework objects
   - LOB storage modules enable a 3-tier setup to handle uploaded
     files and other LOBs. Storage clients talk to trackers who in
     turn provide access to a set of storage nodes. Communication
     is done entirely over HTTP using XML-RPC or SOAP for internal 
     messaging. Only the trackers need access to a shared (preferably
     clustered) database, client and backend modules are standalone.
     Use together with Apache mod_auth_token for time limited 
     token-based authentication in order to prevent unauthorized 
     access or deep-linking.
   - The role based access control (RBAC) module enables access 
     control on user, group and role basis.
   - Log and watchdog module which logs application and HTTP errors
     with environment, context and stacktrace
   - SyncML module for syncing issues and other custom content with 
     mobile devices such as PDA's or cell phones. 
   - Learning management module with course books, quizzes and tests.
     Students work independantly and statistics gathered are viewable 
     by the students themselves and their teachers. 
   - Poll module allows users to design customized online polls, 
     choosing from many types of questions, statistics can be 
     exported to Excel for processing
   - HTML RichText editor for IE and Mozilla/Gecko (SyndRTE) with
     support for image pasting (background uploading)
   - Translation hooks and module for filtering content and 
     translating the interface based on browser negotiated or user
     specifed language setting.
   - Automatic node indexing and searching using the above mentioned 
     indexing strategies
   - Message module supporting GSM SMS messages as well as instant 
     messaging between users
   - Flash based mp3 player for streaming audio to users. Player 
     loads in separete frame so users browse the site and queue 
     additional tracks to the playlist while the audio is playing.
