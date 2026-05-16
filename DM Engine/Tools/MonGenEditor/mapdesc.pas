unit mapdesc;

interface
uses
  Classes, Graphics, SysUtils;
type
  TMapDesc = class
    Name:string;
    mapid:integer;
    Blockmap:string;
  end;
  TGenDesc = class
    Monster:string;
    mapid:integer;
    x:integer;
    y:integer;
    range:integer;
    count:integer;
    time:integer;
    index:integer;
  end;
implementation

end.
 