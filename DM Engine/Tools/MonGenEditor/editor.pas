unit editor;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, SUIForm, SUIButton, Math, WooolMap, ComCtrls,
  SUIListView, StdCtrls, SUIListBox, SUIEdit, SUIComboBox, SUIImagePanel,
  SUIGroupBox, MapDesc, IniFiles, Menus, Contnrs, SUIMainMenu, SUIMgr,
  SUIPopupMenu, SUIURLLabel, SUIScrollBar, define;

type
  TfrmGenEditor = class(TForm)
    ge_frmSkin: TsuiForm;
    tmrDraw: TTimer;
    ge_lvGenList: TsuiListView;
    suiGroupBox1: TsuiGroupBox;
    ge_cbCurMap: TsuiComboBox;
    suiGroupBox2: TsuiGroupBox;
    ge_chkShowName: TsuiCheckBox;
    ge_chkShowAll: TsuiCheckBox;
    ge_cbFilterStyle: TsuiComboBox;
    ge_edFilter: TsuiEdit;
    ge_vGenCount: TsuiSpinEdit;
    ge_vGenTime: TsuiSpinEdit;
    Label4: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    ge_sdMain: TSaveDialog;
    ge_odMain: TOpenDialog;
    suiMainMenu1: TsuiMainMenu;
    F1: TMenuItem;
    miNew: TMenuItem;
    miOpen: TMenuItem;
    miSave: TMenuItem;
    miSaveAs: TMenuItem;
    ge_miExit: TMenuItem;
    miExit: TMenuItem;
    suiBuiltInFileTheme1: TsuiBuiltInFileTheme;
    suiThemeManager1: TsuiThemeManager;
    suiPopupMenu1: TsuiPopupMenu;
    miDeleteGen: TMenuItem;
    Image1: TImage;
    suiGroupBox3: TsuiGroupBox;
    Label2: TLabel;
    ge_vGenTime_e: TsuiSpinEdit;
    ge_vGenCount_e: TsuiSpinEdit;
    Label1: TLabel;
    Label9: TLabel;
    Label3: TLabel;
    ge_vGenX_e: TsuiSpinEdit;
    Label5: TLabel;
    ge_vGenY_e: TsuiSpinEdit;
    ge_vGenRange_e: TsuiSpinEdit;
    suiURLLabel1: TsuiURLLabel;
    suiScrollBar2: TsuiScrollBar;
    suiScrollBar3: TsuiScrollBar;
    ge_cbZoomTimes: TsuiComboBox;
    ge_chkAutoCenter: TsuiCheckBox;
    ge_lbPosition: TLabel;
    ge_lbBlock: TLabel;
    suiButton1: TsuiButton;
    suiPanel1: TsuiPanel;
    ge_pbMapView: TPaintBox;
    suiPanel2: TsuiPanel;
    suiScrollBar1: TsuiScrollBar;
    ge_lbMonsterList: TsuiListBox;
    procedure ge_pbMapViewMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure tmrDrawTimer(Sender: TObject);
    procedure ge_pbMapViewMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure ge_pbMapViewMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormCreate(Sender: TObject);
    procedure ge_lbMonsterListDblClick(Sender: TObject);
    procedure ge_cbCurMapChange(Sender: TObject);
    procedure ge_lvGenListDblClick(Sender: TObject);
    procedure ge_lvGenListSelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure ge_vGenX_eChange(Sender: TObject);
    procedure ge_vGenY_eChange(Sender: TObject);
    procedure ge_vGenRange_eChange(Sender: TObject);
    procedure ge_vGenCount_eChange(Sender: TObject);
    procedure ge_vGenTime_eChange(Sender: TObject);
    procedure miNewClick(Sender: TObject);
    procedure miOpenClick(Sender: TObject);
    procedure ge_cbFilterStyleChange(Sender: TObject);
    procedure miExitClick(Sender: TObject);
    procedure miSaveClick(Sender: TObject);
    procedure miSaveAsClick(Sender: TObject);
    procedure ge_lvGenListKeyUp(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure miDeleteGenClick(Sender: TObject);
    procedure ge_cbZoomTimesChange(Sender: TObject);
    procedure suiButton1Click(Sender: TObject);
  private
    { Private declarations }
    ptFirstPoint:TPoint;
    ptEndPoint:TPoint;
    fFirstClick:Boolean;
    fDragMap:Boolean;
    ptStartDrag:TPoint;
    ptCurDrag:TPoint;
    Curmap:TWooolMap;
    bmBackBuffer:TBitmap;
    ptMapOffset:TPoint;
    TipInfoTimes:integer;
    TipInfo:string;
    CurFile:String;
    ViewGenQueue:TObjectQueue;
    ZoomTimes:integer;
  public
    function PointToMapPosition( x, y:integer ):TPoint;
    function MapPositionToPoint( x, y:integer ):TPoint;
    function Distance( p1,p2:TPoint ):integer;
    procedure DrawGenPoint(monster: string; x, y, range: integer; clr:integer );
    procedure LoadMapList;
    procedure AddGen( monster: string; mapid, x, y, range, count, time: integer; fCheckViewCache:Boolean = true );
    procedure LoadMaps;
    procedure AddMapFile( mapinifile:string );
    procedure SetTipInfo(info: string; Times: integer);
    procedure SetCurFile(filename: string);
    procedure Load(filename: string);
    procedure Save(filename: string);
    procedure CacheViewGen;
    function GetCurMapId: integer;
    procedure UpdateViewCache(index:integer);
    procedure SetZoomTimes(Times:integer);
    procedure CenterMapView(mapx, mapy: integer);
    { Public declarations }
  end;

var
  frmGenEditor: TfrmGenEditor;
  const TITLENAME = '淡抹夕阳怪物刷新编辑器';
implementation

{$R *.dfm}

function TfrmGenEditor.Distance( p1,p2:TPoint ):integer;
begin
  Result := max( abs( p1.X - p2.X ), abs( p1.Y - p2.Y ) );
end;

function TfrmGenEditor.PointToMapPosition( x, y:integer ):TPoint;
begin
  Result.X := x div ZoomTimes - ptMapOffset.X;
  Result.Y := y div ZoomTimes - ptMapOffset.Y;
end;
function TfrmGenEditor.MapPositionToPoint( x, y:integer ):TPoint;
begin
  Result.X := (x + ptMapOffset.X)*ZoomTimes;
  Result.Y := (y + ptMapOffset.Y)*ZoomTimes;
end;

procedure TfrmGenEditor.ge_pbMapViewMouseUp(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  i:integer;
  mapid:integer;
begin
  if Button = mbLeft then
  begin
    if fFirstClick then
    begin
      fFirstClick := false;
      if (ge_vGenCount.Value <= 0) or (ge_vGenTime.Value < 0 ) then
        exit;
      mapid := -1;
      if ge_cbCurMap.ItemIndex >= 0 then
        mapid := TMapDesc(ge_cbCurMap.Items.Objects[ge_cbCurMap.itemindex]).mapid;
      if mapid > 0 then
      begin
        if ge_lbMonsterList.SelCount > 0 then
        begin
          for i := 0 to ge_lbMonsterList.Count -1 do
          begin
            if ge_lbMonsterList.Selected[i] then
              AddGen( ge_lbMonsterList.Items[i], mapid, ptFirstPoint.X, ptFirstPoint.Y,
                Distance( ptFirstPoint, ptEndPoint ), ge_vGenCount.Value, ge_vGenTime.Value );
          end;
        end
        else
          SetTipInfo( '还没有选择怪物，无法添加！', 60 );
      end
      else
        SetTipInfo( '还没有选择地图，无法添加！', 60 );
    end
    else
    begin
      mapid := GetCurMapId;
      if mapid <= 0 then
      begin
        SetTipInfo( '还没有选择地图，无法添加！', 60 );
        exit;
      end;
      if ge_lbMonsterList.SelCount <= 0 then
      begin
        SetTipInfo( '还没有选择怪物，无法添加！', 60 );
        exit;
      end;
      ptFirstPoint := PointToMapPosition( X, Y );
      if (ptFirstPoint.X < 0) or (ptFirstPoint.X >= Curmap.GetWidth) then exit;
      if (ptFirstPoint.Y < 0) or (ptFirstPoint.Y >= Curmap.GetHeight) then exit; 
      fFirstClick := true;
      ptEndPoint := ptFirstPoint;
    end;

  end
  else if Button = mbRight then
  begin
    if fDragMap then
      fDragMap := false;  
  end;
  //tmrDraw.Enabled := fFirstClick;
end;

procedure TfrmGenEditor.tmrDrawTimer(Sender: TObject);
var
  //rcFocuRect:TRect;
  d1,d2:integer;
  CacheGen:TGenDesc;
  rcClipRect:TRect;
  ptMouse:TPoint;
begin
  GetCursorPos( ptMouse );
  ptMouse := ge_pbMapView.ScreenToClient(ptMouse);
  ptMouse := PointToMapPosition( ptMouse.X, ptMouse.Y );
  if (ptMouse.X >= 0) and (ptMouse.Y >= 0) and (ptMouse.X < CurMap.GetWidth ) and (ptMouse.Y < CurMap.GetHeight ) then
  begin
    ge_lbPosition.Caption := 'POS:(' + IntToStr( ptMouse.X ) + ',' + IntToStr( ptMouse.Y ) + ')';
    if CurMap.GetBlockMap.Canvas.Pixels[ptMouse.X, ptMouse.Y] = clrBlocked then
      ge_lbBlock.Caption := '障碍'
    else
      ge_lbBlock.Caption := '无障碍';
  end
  else
  begin
    ge_lbPosition.Caption := '非法坐标';
    ge_lbBlock.Caption := '无';
  end;
  
  //rcFocuRect.TopLeft := ptFirstPoint;
  //rcFocuRect.BottomRight := ptEndPoint;
  d2 := -1;
  if ge_cbCurMap.ItemIndex >= 0 then
  begin
    d2 := TMapDesc(ge_cbCurMap.Items.Objects[ge_cbCurMap.ItemIndex]).mapid;
  end;
  with bmBackBuffer.Canvas do
  begin
    FillRect( ClipRect );
  end;
  if fDragMap then
  begin
    ptMapOffset.X := ptMapOffset.X + (ptCurdrag.X - ptStartDrag.X)div ZoomTimes;
    ptMapOffset.Y := ptMapOffset.Y + (ptCurdrag.Y - ptStartDrag.Y)div ZoomTimes;
    ptStartDrag := ptCurDrag;
  end;
  bmBackBuffer.Canvas.Draw( ptMapOffset.X, ptMapOffset.Y, Curmap.GetBlockMap );
  if fFirstClick then
    DrawGenPoint( '', ptFirstPoint.X, ptFirstPoint.Y, Distance(ptFirstPoint,ptEndPoint), $ffff );

  if ge_chkShowAll.Checked then
  begin
    for d1 := 0 to ViewGenQueue.Count-1 do
    begin
      CacheGen := TGenDesc( ViewGenQueue.Pop );

      DrawGenPoint( CacheGen.Monster, CacheGen.x, CacheGen.y, CacheGen.range, $ff00 );
      ViewGenQueue.Push(CacheGen);
    end;
  {  for d1 := 0 to ge_lvGenList.Items.Count -1 do
    begin
      if ge_lvGenList.ItemIndex <> d1 then
      begin
        if ge_cbFilterStyle.ItemIndex = 1 then
        begin
          if ge_lvGenList.Items[d1].Caption <> ge_edFilter.Text then continue;
        end
        else if ge_cbFilterStyle.ItemIndex = 2 then
        begin
          if ge_lvGenList.Items[d1].Caption = ge_edFilter.Text then continue;
        end;
        if StrToIntDef( ge_lvGenList.Items[d1].SubItems[0], 0 ) <> d2 then continue;
        DrawGenPoint( ge_lvGenList.Items[d1].Caption, StrToIntDef( ge_lvGenList.Items[d1].SubItems[1], 0 ),
                      StrToIntDef( ge_lvGenList.Items[d1].SubItems[2], 0 ),
                      StrToIntDef( ge_lvGenList.Items[d1].SubItems[3], 0 ), $ff00 );
      end;
    end; }
  end;
  if ge_lvGenList.ItemIndex >= 0 then
  begin
    d1 := ge_lvGenList.ItemIndex;
    if StrToIntDef( ge_lvGenList.Items[d1].SubItems[0], 0 ) = d2 then
    begin
      DrawGenPoint( ge_lvGenList.Items[d1].Caption, StrToIntDef( ge_lvGenList.Items[d1].SubItems[1], 0 ),
                    StrToIntDef( ge_lvGenList.Items[d1].SubItems[2], 0 ),
                    StrToIntDef( ge_lvGenList.Items[d1].SubItems[3], 0 ), $f0000ff );
    end;
  end;
  if TipInfoTimes > 0 then
  begin
    dec(TipInfoTimes);
    bmBackBuffer.Canvas.Font.Color := 0;
    bmBackBuffer.Canvas.Font.Style := [];
    SetBkMode( bmBackBuffer.Canvas.Handle, OPAQUE	);
    bmBackBuffer.Canvas.TextOut(4,4,TipInfo);
  end;
  if ZoomTimes = 1 then
    ge_pbMapView.Canvas.Draw(0,0,bmBackBuffer)
  else
  begin
    rcClipRect := ge_pbMapView.Canvas.ClipRect;
    rcClipRect.Right := rcClipRect.Right * ZoomTimes;
    rcClipRect.Bottom := rcClipRect.Bottom * ZoomTimes;
    ge_pbMapView.Canvas.StretchDraw(rcClipRect,bmBackBuffer);
  end;

end;

procedure TfrmGenEditor.ge_pbMapViewMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
  if fFirstClick then
  begin
    ptEndPoint := PointToMapPosition( X, Y );
  end;
  if fDragMap then
  begin
    ptCurDrag.X := X;
    ptCurDrag.Y := Y;
  end;
end;

procedure TfrmGenEditor.ge_pbMapViewMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  if Button = mbRight then
  begin
    fDragMap := true;
    ptStartDrag.X := X;
    ptStartDrag.Y := Y;
    ptCurDrag := ptStartDrag;
  end;
end;

procedure TfrmGenEditor.FormCreate(Sender: TObject);
begin
  ZoomTimes := 1;
  CurMap := TWooolMap.Create;
  ViewGenQueue := TObjectQueue.Create;
  //CurMap.LoadMap( 'E:\Program Files\SHANDA\WOOOL\data\map\2.nmp');
  bmBackBuffer := TBitmap.Create;
  bmBackBuffer.Width := ge_pbMapView.Width;
  bmBackBuffer.Height := ge_pbMapView.Height;
  //SetBkMode( bmBackBuffer.Canvas.Handle, 1 );
  LoadMapList;
  LoadMaps;
  ge_cbCurMapChange(Sender);
  SetCurFile( '' );
end;

procedure TfrmGenEditor.DrawGenPoint(monster: string; x, y,
  range: integer; clr:integer );
var
  ptCenter:TPoint;
  rcFocuRect:TRect;
begin

  if (clr > 0) and ((clr and $f000000) <> 0) then
  begin
    clr := (clr and $ffffff);
    bmBackBuffer.Canvas.Pen.Width := 2;
  end;

  x := x + ptMapOffset.X;
  y := y + ptMapOffset.Y;

  bmBackBuffer.Canvas.Pixels[x, y] := TColor( $ff );
  bmBackBuffer.Canvas.Pixels[x-1, y] := TColor( $ff );
  bmBackBuffer.Canvas.Pixels[x+1, y-1] := TColor( $ff );
  bmBackBuffer.Canvas.Pixels[x-1, y-1] := TColor( $ff );
  bmBackBuffer.Canvas.Pixels[x+1, y] := TColor( $ff );
  bmBackBuffer.Canvas.Pixels[x, y+1] := TColor( $ff );
  bmBackBuffer.Canvas.Pixels[x+1, y+1] := TColor( $ff );
  bmBackBuffer.Canvas.Pixels[x-1, y+1] := TColor( $ff );

  bmBackBuffer.Canvas.Pen.Color := TColor( $ff0000 );

  ptCenter.X := x;
  ptCenter.Y := y;

  bmBackBuffer.Canvas.PenPos := ptCenter;
  bmBackBuffer.Canvas.LineTo( x - 10, y - 10 );
  bmBackBuffer.Canvas.LineTo( x + 10, y - 10 );
  bmBackBuffer.Canvas.LineTo( x, y );

  rcFocuRect.TopLeft.X := x - range;
  rcFocuRect.TopLeft.Y := y - range;

  rcFocuRect.BottomRight.X := x + range;
  rcFocuRect.BottomRight.Y := y + range;
  if clr < 0 then
  begin
    bmBackBuffer.Canvas.DrawFocusRect(rcFocuRect);
    inc(rcFocuRect.Left);
    inc(rcFocuRect.Top);
    dec(rcFocuRect.Right);
    dec(rcFocuRect.Bottom);
    bmBackBuffer.Canvas.DrawFocusRect(rcFocuRect);
  end
  else
  begin

    bmBackBuffer.Canvas.Pen.Color := TColor( clr );
    {bmBackBuffer.Canvas.PenPos := rcFocuRect.TopLeft;
    bmBackBuffer.Canvas.LineTo(rcFocuRect.Left, rcFocuRect.Bottom);
    bmBackBuffer.Canvas.LineTo(rcFocuRect.Right, rcFocuRect.Bottom);
    bmBackBuffer.Canvas.LineTo(rcFocuRect.Right, rcFocuRect.Top);
    bmBackBuffer.Canvas.LineTo(rcFocuRect.Left, rcFocuRect.Top);}
    ptCenter.Y := ptCenter.Y + range;
    bmBackBuffer.Canvas.PenPos := ptCenter;
    ptCenter.Y := ptCenter.Y - range;
    bmBackBuffer.Canvas.LineTo(ptCenter.X-range,ptCenter.Y);
    bmBackBuffer.Canvas.LineTo(ptCenter.X,ptCenter.Y-range);
    bmBackBuffer.Canvas.LineTo(ptCenter.X+range,ptCenter.Y);
    bmBackBuffer.Canvas.LineTo(ptCenter.X,ptCenter.Y+range);
    //bmBackBuffer.Canvas.Pen.Width := 1;
  end;
  if ge_chkShowName.Checked then
  begin
    SetBkMode( bmBackBuffer.Canvas.Handle, TRANSPARENT );
    bmBackBuffer.Canvas.Font.Color := TColor( $0 );
    bmBackBuffer.Canvas.TextOut(x+1,y+1,monster);
    bmBackBuffer.Canvas.Font.Color := TColor( $1fffff );
    bmBackBuffer.Canvas.TextOut(x,y,monster);
  end;
  bmBackBuffer.Canvas.Pen.Width := 1;
end;

procedure TfrmGenEditor.LoadMapList;
var
  slMonster:TStringList;
  i:integer;
begin
  slMonster := TStringList.Create;
  slMonster.LoadFromFile('.\data\BaseMonsterEx.txt' );
  for i := 0 to slMonster.Count -1 do
  begin
    if pchar(slMonster[i])[0] = '@' then
      ge_lbMonsterList.AddItem(pchar(slMonster[i])+1,nil);
  end;
  slMonster.Free;
end;

procedure TfrmGenEditor.AddGen( monster: string; mapid, x, y, range, count, time: integer; fCheckViewCache:Boolean );
var
  CacheGen:TGenDesc;
begin
  ge_lvGenList.AddItem(monster,nil);
  with ge_lvGenList do
  begin
    Items.Item[Items.Count-1].SubItems.Add( IntToStr( mapid ) );
    Items.Item[Items.Count-1].SubItems.Add( IntToStr( x ) );
    Items.Item[Items.Count-1].SubItems.Add( IntToStr( y ) );
    Items.Item[Items.Count-1].SubItems.Add( IntToStr( range ) );
    Items.Item[Items.Count-1].SubItems.Add( IntToStr( count ) );
    Items.Item[Items.Count-1].SubItems.Add( IntToStr( time ) );
  end;
  if not fCheckViewCache then exit;
  ge_lvGenList.ItemIndex := ge_lvGenList.Items.Count -1;

  if mapid = GetCurMapId then
  begin
    if monster = ge_edFilter.Text then
      if ge_cbfilterStyle.ItemIndex = 2 then exit;
    CacheGen := TGenDesc.Create;
    CacheGen.Monster := monster;
    CacheGen.mapid := mapid;
    CacheGen.x := x;
    CacheGen.y := y;
    CacheGen.range := range;
    CacheGen.count := count;
    CacheGen.time := time;
    CacheGen.index := ge_lvGenList.ItemIndex;
    viewGenQueue.Push(CacheGen);
  end;



end;


procedure TfrmGenEditor.LoadMaps;
var
  fhandle:Cardinal;
  wfd:win32_find_data;
begin
  fhandle := FindFirstFile( '.\data\maps\logic\*.ini', wfd );
  if fhandle <> INVALID_HANDLE_VALUE then
  begin
    repeat
      //ge_cbCurMap.AddItem( wfd.cFileName, nil );
      AddMapFile( wfd.cFileName );
    until not FindNextFile( fhandle, wfd );
    //CloseHandle( fhandle );
  end;
  ge_cbCurMap.ItemIndex := 0;
end;

procedure TfrmGenEditor.ge_lbMonsterListDblClick(Sender: TObject);
begin
  if ge_lbMonsterList.ItemIndex <> -1 then
    ge_edFilter.Text := ge_lbMonsterList.Items[ge_lbMonsterList.itemindex];
end;

procedure TfrmGenEditor.AddMapFile( mapinifile:string );
var
  inifile:TIniFile;
  mapdesc:TMapDesc;
begin
  inifile := TIniFile.Create( '.\data\maps\logic\' + mapinifile );
  if inifile = nil then exit;
  mapdesc := TMapDesc.Create;
  mapdesc.Name := inifile.ReadString( 'define', 'name', 'NoName');
  mapdesc.mapid := inifile.ReadInteger( 'define', 'mapid', 16 );
  mapdesc.Blockmap := inifile.ReadString( 'define', 'blockmap', '0' );
  inifile.Free;
  if FileExists( '.\data\maps\physics\' + mapdesc.Blockmap + '.nmp' ) then
    ge_cbCurMap.AddItem( IntToStr( mapdesc.mapid ) + '-' + mapdesc.Name, mapdesc )
  else
    mapdesc.Free;
end;

procedure TfrmGenEditor.ge_cbCurMapChange(Sender: TObject);
var
  index:integer;
  mapdesc:TMapdesc;
begin
  index := ge_cbCurMap.ItemIndex;

  if index >= 0 then
  begin
    mapdesc := TMapDesc( ge_cbCurMap.Items.Objects[index] );
    CurMap.LoadMap( '.\data\maps\physics\' + mapdesc.Blockmap + '.nmp' );
    CacheviewGen;
  end;
  ptMapOffset.X := 0;
  ptMapOffset.Y := 0;
  SetZoomTimes(1);
  CenterMapView( CurMap.GetWidth div 2, CurMap.GetHeight div 2 );
end;

procedure TfrmGenEditor.ge_lvGenListDblClick(Sender: TObject);
var
  i:integer;
  mapid:integer;
  x,y:integer;
  //ptPoint:TPoint;
begin
  if ge_lvGenList.ItemIndex < 0 then exit;
  mapid := StrToIntDef( ge_lvGenList.Items[ge_lvGenList.itemindex].SubItems[0], 0 );
  if mapid <= 0 then exit;
  if ge_cbCurMap.ItemIndex <> -1 then
  begin
    if mapid <> TMapDesc(ge_cbCurMap.Items.Objects[ge_cbCurMap.ItemIndex]).mapid then
    begin
      for i := 0 to ge_cbCurMap.Items.Count -1 do
      begin
        if mapid = TMapDesc(ge_cbCurMap.Items.Objects[i]).mapid then
        begin
          ge_cbCurMap.ItemIndex := i;
          ge_cbCurMapChange( Sender );
          mapid := -1;
          break;
        end;
      end;
    end;
  end;
  if mapid = -1 then
  begin
    x := StrToIntDef( ge_lvGenList.Items[ge_lvGenList.itemindex].SubItems[1], 0 );
    y := StrToIntDef( ge_lvGenList.Items[ge_lvGenList.itemindex].SubItems[2], 0 );
  //SetZoomTimes(1);
    CenterMapView( x, y );
  end;


end;

procedure TfrmGenEditor.ge_lvGenListSelectItem(Sender: TObject;
  Item: TListItem; Selected: Boolean);
begin
  ge_vGenX_e.Value := StrToIntDef( Item.SubItems[1], 0 );
  ge_vGenY_e.Value := StrToIntDef( Item.SubItems[2], 0 );
  ge_vGenCount_e.Value := StrToIntDef( Item.SubItems[4], 0 );
  ge_vGenTime_e.Value := StrToIntDef( Item.SubItems[5], 0 );
  ge_vGenRange_e.Value := StrToIntDef( Item.SubItems[3], 0 );
  if ge_chkAutoCenter.Checked then
    CenterMapView( ge_vGenX_e.Value, ge_vGenY_e.Value );
  //Selected := true;
end;

procedure TfrmGenEditor.ge_vGenX_eChange(Sender: TObject);
var
  index:integer;
begin
  index := ge_lvGenList.ItemIndex;
  if index <> -1 then
  begin
    ge_lvGenList.Items[index].SubItems[1] := IntToStr( ge_vGenX_e.Value );
    UpdateViewCache( index );
  end;
end;

procedure TfrmGenEditor.ge_vGenY_eChange(Sender: TObject);
var
  index:integer;
begin
  index := ge_lvGenList.ItemIndex;
  if index <> -1 then
  begin
    ge_lvGenList.Items[index].SubItems[2] := IntToStr( ge_vGenY_e.Value );
    UpdateViewCache( index );
  end;
end;

procedure TfrmGenEditor.ge_vGenRange_eChange(Sender: TObject);
var
  index:integer;
begin
  index := ge_lvGenList.ItemIndex;
  if index <> -1 then
  begin
    ge_lvGenList.Items[index].SubItems[3] := IntToStr( ge_vGenRange_e.Value );
    UpdateViewCache( index );
  end;
end;

procedure TfrmGenEditor.ge_vGenCount_eChange(Sender: TObject);
var
  index:integer;
begin
  index := ge_lvGenList.ItemIndex;
  if index <> -1 then
  begin
    ge_lvGenList.Items[index].SubItems[4] := IntToStr( ge_vGenCount_e.Value );
    UpdateViewCache( index );
  end;
end;

procedure TfrmGenEditor.ge_vGenTime_eChange(Sender: TObject);
var
  index:integer;
begin
  index := ge_lvGenList.ItemIndex;
  if index <> -1 then
  begin
    ge_lvGenList.Items[index].SubItems[5] := IntToStr( ge_vGenTime_e.Value );
    UpdateViewCache( index );
  end;
end;

procedure TfrmGenEditor.SetTipInfo(info: string; Times: integer);
begin
  TipInfo := info;
  TipInfoTimes := Times;
end;

procedure TfrmGenEditor.miNewClick(Sender: TObject);
begin
  ge_lvGenList.Clear;
end;

procedure TfrmGenEditor.SetCurFile(filename: string);
begin
  CurFile := filename;
  if curfile = '' then
    Caption := TITLENAME + '－' + '未命名'
  else
    Caption := TITLENAME + '－' + CurFile;
  ge_frmSkin.Caption := Caption;
end;

procedure TfrmGenEditor.miOpenClick(Sender: TObject);
begin
  if ge_odMain.Execute then
  begin
    if ge_odMain.FileName <> '' then
    begin
      Load( ge_odMain.FileName );
    end;
  end;
end;

procedure TfrmGenEditor.Load(filename: string);
var
  slFile:TStringList;
  slLine:TStringList;
  i:integer;
  spe,wht:TSysCharSet;
begin
  ge_lvGenList.Clear;
  ge_lbMonsterList.ClearSelection;
  slFile := TStringList.Create;
  slLine := TStringList.Create;
  spe := ['/'];
  wht := [' ', char($9) ];
  SetCurFile( filename );
  slFile.LoadFromFile(filename);
  for i := 0 to slFile.Count-1 do
  begin
    slLine.Clear;
    if pchar( slFile[i] )[0] = '#' then continue;
    ExtractStrings( spe, wht, pchar( slFile[i] ), slLine );
    if slLine.Count >= 7 then
    begin
      AddGen( slLine[0], StrToIntDef( slLine[1], 0 ), StrToIntDef( slLine[2], 0 ),
      StrToIntDef( slLine[3], 0 ), StrToIntDef( slLine[4], 0 ),
      StrToIntDef( slLine[5], 0 ), StrToIntDef( slLine[6], 0 ), false );

    end;
  end;
  slFile.Free;
  slLine.Free;
  CacheViewGen;
  SetZoomTimes(1);
  //Application.ExeName
end;

procedure TfrmGenEditor.Save(filename: string);
var
  slFile:TStringList;
  i:integer;
begin
  SetCurFile( filename );
  slFile := TStringList.Create;
  for i := 0 to ge_lvGenList.Items.Count - 1 do
  begin
    slFile.Add( ge_lvGenList.Items[i].Caption + '/' + ge_lvGenList.Items[i].SubItems[0] + '/' +
                ge_lvGenList.Items[i].SubItems[1] + '/' + ge_lvGenList.Items[i].SubItems[2] + '/' +
                ge_lvGenList.Items[i].SubItems[3] + '/' + ge_lvGenList.Items[i].SubItems[4] + '/' +
                ge_lvGenList.Items[i].SubItems[5] );
  end;
  slFile.SaveToFile( filename );
  slFile.Free;
end;

procedure TfrmGenEditor.ge_cbFilterStyleChange(Sender: TObject);
begin
//  if ge_edFilter.Text = '' then exit;
  CacheViewGen;
end;

procedure TfrmGenEditor.CacheViewGen;
var
  CacheGen:TGenDesc;
  i,mapid,count:integer;
  monster: string;
  fView: Boolean;
begin
  fView := false;
  if ge_cbFilterStyle.ItemIndex = 0 then
    monster := ''
  else if ge_cbFilterStyle.ItemIndex = 1 then
  begin
    monster := ge_edFilter.Text;
    fView := true;
  end
  else
  begin
    monster := ge_edFilter.Text;
    fView := false;
  end;
  
  if ViewGenQueue.Count > 0 then
  begin
    count := ViewGenQueue.Count;

    for i := 0 to count-1 do
    begin
      CacheGen := TGenDesc(ViewGenQueue.Pop);
      CacheGen.Free;
    end;
  end;
  mapid := GetCurMapId;
  if mapid >= 0 then
  begin
    for i := 0 to ge_lvGenList.Items.Count -1 do
    begin
      //  检测地图号
      if mapid = StrToIntDef( ge_lvGenList.Items[i].SubItems[0], 0 ) then
      begin
        if fView then
        begin
          if monster <> ge_lvGenList.Items[i].Caption then continue
        end
        else
          if monster = ge_lvGenList.Items[i].Caption then continue;
        CacheGen := TGenDesc.Create;
        CacheGen.Monster := ge_lvGenList.Items[i].Caption;
        CacheGen.mapid := mapid;
        CacheGen.x := StrToIntDef( ge_lvGenList.Items[i].SubItems[1], 0 );
        CacheGen.y := StrToIntDef( ge_lvGenList.Items[i].SubItems[2], 0 );
        CacheGen.range := StrToIntDef( ge_lvGenList.Items[i].SubItems[3], 0 );
        CacheGen.count := StrToIntDef( ge_lvGenList.Items[i].SubItems[4], 0 );
        CacheGen.time := StrToIntDef( ge_lvGenList.Items[i].SubItems[5], 0 );
        CacheGen.index := i;
        ViewGenQueue.Push( CacheGen );
      end;
    end;
  end;
end;

function TfrmGenEditor.GetCurMapId: integer;
begin
  Result := -1;
  if ge_cbCurMap.ItemIndex <> -1 then
    Result := TMapDesc( ge_cbCurMap.Items.Objects[ge_cbCurMap.ItemIndex] ).mapid;
end;

procedure TfrmGenEditor.UpdateViewCache(index:integer);
var
  CacheGen:TGenDesc;
  i,count:integer;
begin
  if ViewGenQueue.Count > 0 then
  begin
    count := ViewGenQueue.Count;
    //CacheGen := TGenDesc(ViewGenQueue.Pop);
    for i := 0 to count -1 do
    begin
      CacheGen := TGenDesc( ViewGenQueue.Pop );
      ViewGenQueue.Push(CacheGen);
      if CacheGen.index = index then
      begin
        if (index >= 0) and (index < ge_lvGenList.Items.Count ) then
        begin
          CacheGen.x := StrToIntDef( ge_lvGenList.Items[index].SubItems[1], 0 );
          CacheGen.y := StrToIntDef( ge_lvGenList.Items[index].SubItems[2], 0 );
          CacheGen.range := StrToIntDef( ge_lvGenList.Items[index].SubItems[3], 0 );
          CacheGen.count := StrToIntDef( ge_lvGenList.Items[index].SubItems[4], 0 );
          CacheGen.time := StrToIntDef( ge_lvGenList.Items[index].SubItems[5], 0 );
        end;
        exit;
      end;
    end;
  end;
end;

procedure TfrmGenEditor.miExitClick(Sender: TObject);
begin
  Close;
end;

procedure TfrmGenEditor.miSaveClick(Sender: TObject);
begin
  if CurFile <> '' then
    Save( CurFile )
  else
  begin
    if ge_sdMain.Execute then
    begin
      if ge_sdMain.FileName <> '' then
      begin
        Save( ge_sdMain.FileName );
      end;
    end;
  end;
end;

procedure TfrmGenEditor.miSaveAsClick(Sender: TObject);
begin
    if ge_sdMain.Execute then
    begin
      if ge_sdMain.FileName <> '' then
      begin
        Save( ge_sdMain.FileName );
      end;
    end;
end;

procedure TfrmGenEditor.ge_lvGenListKeyUp(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if (Key = vk_Delete) and ( ge_lvGenList.ItemIndex >= 0 ) then
  begin
    ge_lvGenList.Items.Delete(ge_lvGenList.ItemIndex);
    ge_lvGenList.Refresh;
    CacheViewGen;
  end;
end;

procedure TfrmGenEditor.miDeleteGenClick(Sender: TObject);
begin
  if ge_lvGenList.ItemIndex >= 0 then
  begin
    ge_lvGenList.Items.Delete(ge_lvGenList.ItemIndex);
    ge_lvGenList.Refresh;
    CacheViewGen;
  end;

end;

procedure TfrmGenEditor.ge_cbZoomTimesChange(Sender: TObject);
var
  ptCenter:TPoint;
begin
  ptCenter.X := ge_pbMapView.Width div 2;
  ptCenter.Y := ge_pbMapView.Height div 2;
  ptCenter := PointToMapPosition( ptCenter.X, ptCenter.Y );
  ZoomTimes := ge_cbZoomTimes.ItemIndex + 1;
  CenterMapView( ptCenter.X, ptCenter.Y );
end;

procedure TfrmGenEditor.SetZoomTimes(Times:integer);
begin
  if (Times < 1) or( Times > 2 ) then exit;
  ge_cbZoomTimes.ItemIndex := Times-1;
  ZoomTimes := Times;
end;

procedure TfrmGenEditor.CenterMapView(mapx, mapy: integer);
var
  cx,cy:integer;
begin
  cx := (ge_pbMapView.Width div ZoomTimes)div 2;
  cy := (ge_pbMapView.Height div ZoomTimes)div 2;
  ptMapOffset.X := cx - mapx;
  ptMapOffset.Y := cy - mapy;
end;

procedure TfrmGenEditor.suiButton1Click(Sender: TObject);
begin
  CenterMapView( CurMap.GetWidth div 2, CurMap.GetHeight div 2 );
end;

end.
