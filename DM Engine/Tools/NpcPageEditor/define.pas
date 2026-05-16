unit define;

interface

uses
  Windows, SysUtils, Variants, Classes, Graphics;
type
  TDrawPage = class
  private
    Canvas:TCanvas;
    x:integer;
    y:integer;
    state:integer;
    curClr:TColor;
    buffer:array[0..65535] of char;
    bufferptr:integer;
    fUnderLine:Boolean;
  public
    procedure ParsePageToCanvas( rCanvas:TCanvas; rx,ry:integer; strPage:string );
  protected
    procedure SetState( istate:integer );
    procedure SetColor( clrSet:TColor );
    procedure DrawBuffer();
  end;
 { TPageParser = class
  private
    ParsedPage:string;
    slSituation:TStringList;
  public
    procedure ParsePageLines( slPageLines:TStringList );
    function GetParsedPage():string;
  end;}
function GetColor( clrName:string):TColor;
implementation

function GetColor( clrName:string):TColor;
begin
  clrName := strupper( pchar(clrName));
  if clrName = 'RED' then
  begin
    Result := clRed;
    exit;
  end;
  if clrName = 'GREEN' then
  begin
    Result := clGreen;
    exit;
  end;
  if clrName = 'BLUE' then
  begin
    Result := clBlue;
    exit;
  end;
  if clrName = 'YELLOW' then
  begin
    Result := clYellow;
    exit;
  end;
  if clrName = 'BLACK' then
  begin
    Result := clBlack;
    exit;
  end;
  if clrName = 'WHITE' then
  begin
    Result := clWhite;
    exit;
  end;
  if clrName = 'PURPLE' then
  begin
    Result := clPurple;
    exit;
  end;
  if clrName = 'ORANGE' then
  begin
    Result := RGB(253,156,49);
    exit;
  end;
  if clrName = 'GRAY' then
  begin
    Result := clGray;
    exit;
  end;
  TRY
    Result := StrToInt( '$' + clrName );
    Result := RGB( (Result and $ff0000) shr 16, (Result and $ff00)shr 8, Result and $ff );
  EXCEPT
    Result := $ffffff;
  END;
end;

procedure TDrawPage.SetState( istate:integer );
begin
  state := istate;
  if state = 0 then
    SetColor( $ffffff )
  else if state = 1 then
    SetColor( $ffff );
end;

procedure TDrawPage.SetColor( clrSet:TColor );
begin
  curClr := clrSet;
end;

procedure TDrawPage.DrawBuffer();
var
  width:integer;
  strTemp:string;
begin
  if bufferptr > 0 then
  begin
    if (state <> 4) and (state <> 2 ) then
    begin
      buffer[bufferptr] := char(0);
      strTemp := buffer;
      width := Canvas.TextWidth( strTemp );
      if fUnderLine then
        Canvas.Font.Style := [fsUnderline];
      Canvas.Font.Color := curClr;
      Canvas.TextOut( x, y, strTemp );
      if fUnderLine then
        Canvas.Font.Style := [];
      inc( x, width );
    end;
    bufferptr := 0;
  end;
end;
{
0- normal
1- after <
2- after =
3- color text
4- not draw
}
procedure TDrawPage.ParsePageToCanvas( rCanvas:TCanvas; rx,ry:integer; strPage:string );
var
  pPage:pchar;
  i,pagelen:integer;
  strTemp:string;
  fDisableLine:boolean;
begin
  pPage := pchar(strPage);
  x := rx;
  y := ry;
  Canvas := rCanvas;
  SetBkMode( Canvas.Handle, TRANSPARENT );
  pagelen := strlen( pPage );
  SetState( 0 );
  SetColor( $ffffff );
  fDisableLine := false;
  for i := 0 to pagelen -1 do
  begin
    if state = 2 then
    begin
      if (pPage[i] = ' ') or (pPage[i] = char(9)) then
      begin
        buffer[bufferptr] := char(0);
        curClr := GetColor( buffer );
        bufferptr := 0;
        SetState( 3 );
        continue;
      end;
      if pPage[i] = '>' then
      begin
        bufferptr := 0;
        SetState( 0 );
        continue;
      end;
    end
    else if state = 3 then
    begin
      if pPage[i] = '>' then
      begin
        DrawBuffer;
        SetState( 0 );
        continue;
      end;
    end
    else if state = 1 then
    begin
      if pPage[i] = '/' then
      begin
        fUnderLine := true;
        DrawBuffer;
        fUnderLine := false;
        SetState( 4 );
        continue;
      end
      else if pPage[i] = '=' then
      begin
        buffer[bufferptr] := char(0);
        strTemp := strupper(buffer);
        if strTemp = 'COLOR' then
        begin
          bufferptr := 0;
          SetState(2);
          continue;
        end;
      end
      else if pPage[i] = '>' then
      begin
        DrawBuffer;
        SetState( 0 );
        continue;
      end;
    end
    else
    begin
      if pPage[i] = '<' then
      begin
        DrawBuffer;
        SetState( 1 );
        continue;
      end
      else if pPage[i] = '>' then
      begin
        DrawBuffer;
        SetState( 0 );
        continue;
      end;

    end;

    if (pPage[i] = #$0a) or (pPage[i] = #$0d) then continue;
    if pPage[i] = '\' then
    begin
      DrawBuffer;
      inc( y, 16 );
      x := rx;
      continue;
    end;
    buffer[bufferptr] := pPage[i];
    inc(bufferptr);
  end;
  DrawBuffer;
end;

end.
