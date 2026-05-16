unit WooolMap;

interface

uses
  Classes, Graphics, SysUtils, define;

type
  TWooolMap = class
  private
    bmBlockMap:TBitmap;
    Height:integer;
    Width:integer;
  public
    function GetBlockMap:TBitmap;
    function GetWidth:integer;
    function GetHeight:integer;

    function LoadMap( const mapfile:string ):Boolean;

  end;

implementation
function TWooolMap.GetBlockMap:TBitmap;
begin
  Result := bmBlockMap;
end;

function TWooolMap.GetWidth:integer;
begin
  Result := Width;
end;

function TWooolMap.GetHeight:integer;
begin
  Result := Height;
end;

function TWooolMap.LoadMap( const mapfile:string ):Boolean;
var
  fmap:TFileStream;
  Startofs:integer;
  Flag:Byte;
  x,y:integer;
  temp1:integer;
  //temp2:integer;
begin
  Result := FALSE;
  fmap := TFileStream.Create( mapfile, fmOpenRead );
  if fmap = nil then exit;
  if bmBlockMap <> nil then bmBlockMap.Free;
  bmBlockMap := TBitmap.Create;
  fmap.Read(Startofs,4);
  fmap.Seek(8,soFromBeginning);
  fmap.Read(Width,4);
  fmap.Read(Height,4);
  fmap.Seek(Startofs,soFromBeginning);
  Startofs := Width * Height;
  bmBlockMap.Width := Width;
  bmBlockMap.Height := Height;
  x := 0;
  y := 0;
  while Startofs <> 0 do
  begin
    dec( StartOfs );
    fmap.Read(Flag, 1 );
    if (Flag and 1) = 1 then
      bmBlockMap.Canvas.Pixels[x,y] := TColor( clrBlocked )
    else
      bmBlockMap.Canvas.Pixels[x,y] := TColor( clrNoBlocked );
    if (Flag and 2) > 0 then
      fmap.Read(temp1, 2 );
    if (Flag and 4) > 0 then
      fmap.Read(temp1, 2 );
    if (Flag and 8) > 0 then
      fmap.Read(temp1, 4 );
    if (Flag and 16) > 0 then
      fmap.Read(temp1, 1 );
    if (Flag and 32) > 0 then
      fmap.Read(Flag, 1 );
    if (Flag and 64) > 0 then
      fmap.Read(Flag, 1 );
    if (Flag and 128) > 0 then
      fmap.Read(Flag, 1 );
    inc( x );
    if x >= Width then
    begin
      x := 0;
      inc( y );
    end;

  end;
  fmap.Free;
  Result := true;
end;

end.
