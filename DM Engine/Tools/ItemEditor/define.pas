unit define;

interface
uses
  Classes,SysUtils,Windows;
type
  TItemPropSetting = record
    used:Boolean;
    name:string;
    offsetvalue:integer;
  end;
  PItemPropSetting = ^TItemPropSetting;
  TItemClass = class( TObject )
  protected
    PropSetting:array[0..11] of TItemPropSetting;
    mName:string;
    mStdMode:Byte;
    mType:Byte;
    mShapeList:TStringList;
    function GetPropSetting( index:integer ):PItemPropSetting;
  public
    constructor Create;
    destructor Destroy; override;
    procedure Copy( itemClass:TItemClass );

    property Name:string read mName write mName;
    property StdMode:Byte read mStdMode write mStdMode;
    property DataType:Byte read mType write mType;
    property Shapes:TStringList read mShapeList;
    property Props[index:integer]:PItemPropSetting read GetPropSetting;
    function ParseShapes( lines:TStringList ):Boolean;
    function GetShapeValue( sShape:string ):integer;

    function ToString:string;
    function FromString( S:String ):Boolean;
  end;

  TItem = class( TObject )
  protected
    mClassName:String;
    mName:String;
    mShape:Byte;
    mWeight:Byte;
    mImage:Word;
    mFlag:DWORD;
    mPropData:array[0..11] of integer;
    function GetPropValue( index: integer ):integer;
    procedure SetPropValue( index, value:integer );
    function GetFlag( index: integer ):Boolean;
    procedure SetFlag( index: integer; bFlag:Boolean );
    
  public
    constructor Create;
    destructor Destroy; override;
    procedure Copy( item:TItem );
    function ToString:string;
    function FromString( S:String ):Boolean;

    property Name:string read mClassName write mClassName;
    property ViewName:string read mName write mName;
    property Props[index:integer]:Integer read GetPropValue write SetPropValue;
    property Flags[index:integer]:Boolean read GetFlag write SetFlag;
    property ImageIndex:Word read mImage write mImage;
    property Weight:Byte read mWeight write mWeight;
    property Shape:Byte read mShape write mShape;
  end;
  procedure Init;
  function LoadItemClasses( sFile:string ):Boolean;
  function SaveItemClasses( sFile:string ):Boolean;
var
  gItemClasses:TStringList;
implementation

procedure Init;
begin
  gItemClasses := TStringList.Create;
end;
function LoadItemClasses( sFile:string ):Boolean;
begin
end;
function SaveItemClasses( sFile:string ):Boolean;
begin
end;



constructor TItemClass.Create;
begin
  mShapeList := TStringList.Create;
  ZeroMemory( @PropSetting, sizeof( PropSetting ) );
end;

destructor TItemClass.Destroy;
begin
  mShapeList.Free;
  inherited;
end;

function TItemClass.GetPropSetting( index:integer ):PItemPropSetting;
begin
  if (index < 0) or (index > 11) then
  begin
    Result := nil;
    exit;
  end;
  Result := @PropSetting[index];
end;

function TItemClass.ParseShapes( lines:TStringList ):Boolean;
var
  i:integer;
  slSplit:TStringList;
  whtSet:TSysCharSet;
  splSet:TSysCharSet;
  //s:String;
begin
  mShapeList.Clear;
  whtSet := [#9, ' '];
  splSet := ['='];
  slSplit := TStringList.Create;
  for i := 0 to lines.Count-1 do
  begin
    slSplit.Clear;
    ExtractStrings( splSet, whtSet, pchar(lines[i]),TStrings(slSplit) );
    if slSplit.Count = 2 then
    begin
      mShapeList.AddObject( Trim(slSplit[0]), TObject(StrToIntDef( Trim(slSplit[1]), 0 )));
    end;
  end;
  slSplit.Free;
  if (mShapeList.Count = 0) or (integer(mShapeList.Objects[0]) <> 0) then
  begin
    mShapeList.InsertObject( 0, '∆’Õ®', TObject(0));
  end;
  Result := true;
end;

function TItemClass.GetShapeValue( sShape:string ):integer;
var
  index:integer;
begin
  index := mShapeList.IndexOf(sShape);
  if index = -1 then
    Result := 0
  else
    Result := integer(mShapeList.Objects[index]);
end;

function TItemClass.ToString:string;
begin
end;

function TItemClass.FromString( S:String ):Boolean;
begin
end;

procedure TItemClass.Copy( itemClass:TItemClass );
begin
end;

function TItem.GetPropValue( index: integer ):integer;
begin
  Result := 0;
  if (index < 0 ) or ( index > 11 ) then exit;
  Result := mPropData[index];
end;

procedure TItem.SetPropValue( index, value:integer );
begin
  if (index < 0 ) or ( index > 11 ) then exit;
  mPropData[index] := value;
end;

function TItem.GetFlag( index: integer ):Boolean;
var
  f:dword;
begin
  Result := false;
  if( index < 0 ) or ( index > 31 ) then exit;
  f := 1 shl index;
  if (f and mFlag) = 0 then exit;
  Result := true;
end;

procedure TItem.SetFlag( index: integer; bFlag:Boolean );
var
  f:dword;
begin
  if( index < 0 ) or ( index > 31 ) then exit;
  f := 1 shl index;
  if ( f and mFlag ) = 0 then
  begin
    if bFlag then
      mFlag := mFlag or f;
  end
  else
  begin
    if not bFlag then
      mFlag := mFlag xor f;
  end;
end;

constructor TItem.Create;
begin

end;

destructor TItem.Destroy;
begin
  inherited;
end;

function TItem.ToString:string;
begin
  
end;

function TItem.FromString( S:String ):Boolean;
begin
end;

procedure TItem.Copy( item:TItem );
begin
end;

end.
