Map compiling utilities for Quake, version 2
------------
Originally by id software
Portions copyright Alexander Malmberg <alexander@malmberg.org>


Here are versions of qbsp, vis, and light for Quake. Use at your own risk. 
They use huge amounts of memory. You can find source code and binaries
for DOS/Windows (DJGPP) and Linux-i386-glibc, as well as new versions,
at http://quest-ed.sourceforge.net/ .

Additions:
----------
*  qbsp and vis now support detail brushes. Quest v2.4 will have support
  for setting this like surface attributes in Quake 2, but for now, and
  for other editors that don't support this directly, place any detail 
  brushes in an entity with classname 'am_detail'. qbsp will mark these 
  brushes as detail, move them to the worldspawn entity and delete this 
  entity. Supporting detail brushes required changes to the portal file 
  format, so qbsp and vis found here won't work with versions of qbsp 
  and vis that don't support detail brushes.
  
   qbsp also supports hint and skip brushes. They work like in Quake 2,
  i.e. a face with the HINT texture will force a BSP split and a face
  with the SKIP texture will be ignored completely. Included is
  hintskip.wad, a .wad file with the HINT and SKIP textures. If your
  editor can't load multiple .wad files, run 'addwadhs foo.wad' to
  add the HINT and SKIP texture to foo.wad (backup first, though).

   For those that don't know, detail brushes are just like ordinary brushes
  with the exception that they aren't used in visibility calculations. Thus,
  by marking small brushes that don't block visibility much as detail brushes
  you'll have much lower vis times, and the generated PVS will (unless you
  marked a large brush detail) be as good as before. For more information
  on detail, hint and skip brushes, you can go to http://www.gamedesign.net/ 
  and read their tutorials on these subjects. Although they're targetted at 
  Quake 2, it works (mostly) the same way here.


   For those who want to add support for detail brushes to their favorite
  editor: qbsp will consider any brushes with the word 'detail' after
  any plane definition to be a detail brush, e.g.:
  ( 0 0 0 ) ( 1 0 0 ) ( 0 1 0 ) SKY1 0 0 0 1 1 detail
  
   Additionally, any brushes in a 'am_detail' entity will be flagged as
  detail brushes and moved to worldspawn.
   

*  The included qbsp can read both the old map format and Quest's new map 
  format.
*  qbsp produces a better .pts file. It won't work for loading into Quake,
  but it's much better when loaded into Quest.
*  light and vis have progress meters like the qbsp3, qvis3, and qrad3 I've
  released. The number to the left of the dots is the amount of work per
  dot.

These programs were originally written by id software, I've only modified
them. Still, I'll try to fix any bugs and answer any questions I get.

- Alexander Malmberg <alexander@malmberg.org>
