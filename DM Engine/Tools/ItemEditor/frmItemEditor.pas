unit frmItemEditor;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, SUIForm, sglfile, SUIButton, StdCtrls, SUIComboBox,
  SUIImagePanel, SUIGroupBox, SUIEdit, CheckLst, SUICheckListBox,
  SUIListBox, ItemClassEd, SUIMgr;

type
  TPropEditGroup = record
    PLabel:^TLabel;
    PEdit:^TsuiSpinEdit;
  end;
  TfrmMainEditor = class(TForm)
    suiForm1: TsuiForm;
    suiButton1: TsuiButton;
    Image1: TImage;
    Timer1: TTimer;
    suiGroupBox1: TsuiGroupBox;
    suiEdit1: TsuiEdit;
    Label1: TLabel;
    Label2: TLabel;
    suiEdit2: TsuiEdit;
    suiComboBox2: TsuiComboBox;
    Label3: TLabel;
    suiSpinEdit1: TsuiSpinEdit;
    Label4: TLabel;
    Label5: TLabel;
    suiSpinEdit2: TsuiSpinEdit;
    Label6: TLabel;
    suiComboBox3: TsuiComboBox;
    suiComboBox1: TsuiComboBox;
    Label7: TLabel;
    suiGroupBox2: TsuiGroupBox;
    suiCheckListBox1: TsuiCheckListBox;
    suiListBox1: TsuiListBox;
    suiSpinEdit3: TsuiSpinEdit;
    suiThemeManager1: TsuiThemeManager;
    ie_sedSp: TsuiSpinEdit;
    ie_sedAc1: TsuiSpinEdit;
    ie_sedAc2: TsuiSpinEdit;
    ie_sedMac1: TsuiSpinEdit;
    ie_sedMac2: TsuiSpinEdit;
    ie_sedDc1: TsuiSpinEdit;
    ie_sedDc2: TsuiSpinEdit;
    ie_sedMc1: TsuiSpinEdit;
    ie_sedMc2: TsuiSpinEdit;
    ie_sedSc1: TsuiSpinEdit;
    ie_sedSc2: TsuiSpinEdit;
    ie_sedDura: TsuiSpinEdit;
    ie_lbSp: TLabel;
    ie_lbAc1: TLabel;
    ie_lbAc2: TLabel;
    ie_lbMac1: TLabel;
    ie_lbMac2: TLabel;
    ie_lbDc1: TLabel;
    ie_lbDc2: TLabel;
    ie_lbMc1: TLabel;
    ie_lbMc2: TLabel;
    ie_lbSc1: TLabel;
    ie_lbSc2: TLabel;
    ie_lbDura: TLabel;
    procedure FormCreate(Sender: TObject);
    procedure suiButton1Click(Sender: TObject);
    procedure suiComboBox1DrawItem(Control: TWinControl; Index: Integer;
      Rect: TRect; State: TOwnerDrawState);
    procedure FormPaint(Sender: TObject);
    procedure suiComboBox1Change(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure suiSpinEdit3Change(Sender: TObject);
    
  private
    { Private declarations }
    ItemImageFile:TSGLFile;
    DnItemImageFile:TSGLFile;
    FrameIndex:integer;
    PropEditGroups:array[0..11] of TPropEditGroup;
  public
    { Public declarations }
  end;

var
  frmMainEditor: TfrmMainEditor;

implementation

{$R *.dfm}

procedure TfrmMainEditor.FormCreate(Sender: TObject);
var
  i:integer;
begin
  ItemImageFile := TSglFile.Create;
  if not ItemImageFile.Open('.\\Items.sgl') then
  begin
    ItemImageFile.Free;
    ItemImageFile := nil;
  end
  else
  begin
    ItemImageFile.ImageIndex := 0;
    for i := 0 to ItemImageFile.ImageCount -1 do
    begin
      suiComboBox1.AddItem( 'ox', nil );
    end;
    suiComboBox1.ItemIndex := 0;
  end;
  DnItemImageFile := TSglFile.Create;
  if not DnItemImageFile.Open( '.\\DnItems.sgl' ) then
  begin
    DnItemImageFile.Free;
    DnItemImageFile := nil;
  end;

  PropEditGroups[0].PLabel := @ie_lbSp;
  PropEditGroups[0].PEdit := @ie_sedSp;
  PropEditGroups[1].PLabel := @ie_lbAc1;
  PropEditGroups[1].PEdit := @ie_sedAc1;
  PropEditGroups[2].PLabel := @ie_lbAc2;
  PropEditGroups[2].PEdit := @ie_sedAc2;
  PropEditGroups[3].PLabel := @ie_lbMac1;
  PropEditGroups[3].PEdit := @ie_sedMac1;
  PropEditGroups[4].PLabel := @ie_lbMac2;
  PropEditGroups[4].PEdit := @ie_sedMac2;
  PropEditGroups[5].PLabel := @ie_lbDc1;
  PropEditGroups[5].PEdit := @ie_sedDc1;
  PropEditGroups[6].PLabel := @ie_lbDc2;
  PropEditGroups[6].PEdit := @ie_sedDc2;
  PropEditGroups[7].PLabel := @ie_lbMc1;
  PropEditGroups[7].PEdit := @ie_sedMc1;
  PropEditGroups[8].PLabel := @ie_lbMc2;
  PropEditGroups[8].PEdit := @ie_sedMc2;
  PropEditGroups[9].PLabel := @ie_lbSc1;
  PropEditGroups[9].PEdit := @ie_sedSc1;
  PropEditGroups[10].PLabel := @ie_lbSc2;
  PropEditGroups[10].PEdit := @ie_sedSc2;
  PropEditGroups[11].PLabel := @ie_lbDura;
  PropEditGroups[11].PEdit := @ie_sedDura;
end;

procedure TfrmMainEditor.suiButton1Click(Sender: TObject);
begin
  //inc(ItemImageFile.ImageIndex);
  frmItemClassEd.Show;
end;

procedure TfrmMainEditor.suiComboBox1DrawItem(Control: TWinControl; Index: Integer;
  Rect: TRect; State: TOwnerDrawState);
begin
    suiComboBox1.Canvas.Brush.Color := TColor( $ffffff );
    suiComboBox1.Canvas.FillRect(rect);
    ItemImageFile.DrawToCanvas( Index, 0, suiComboBox1.Canvas, rect.Left+4, rect.Top+4 );
end;



procedure TfrmMainEditor.FormPaint(Sender: TObject);
begin
  if DnItemImageFile <> nil then
  begin
    DnItemImageFile.DrawToCanvas( suiComboBox1.ItemIndex, 0, Image1.Canvas, 0, 0 );
  end;
end;

procedure TfrmMainEditor.suiComboBox1Change(Sender: TObject);
begin
  if DnItemImageFile <> nil then
  begin
    DnItemImageFile.DrawToCanvas( suiComboBox1.ItemIndex, 0, Image1.Canvas, 0, 0 );
  end;
  suiSpinEdit3.Value := suiComboBox1.ItemIndex;
end;

procedure TfrmMainEditor.Timer1Timer(Sender: TObject);
begin
  inc( FrameIndex );
  if DnItemImageFile <> nil then
  begin
    DnItemImageFile.DrawToCanvas( suiComboBox1.ItemIndex, 0, Image1.Canvas, 0, 0 );
  end;
end;

procedure TfrmMainEditor.suiSpinEdit3Change(Sender: TObject);
var
  index :integer;
begin
  if suiComboBox1.ItemIndex = suiSpinEdit3.Value then exit;
  index := suiSpinEdit3.Value;
  if (index < 0 )or(index >= suiComboBox1.Items.Count) then
    suiSpinEdit3.Value := suiComboBox1.ItemIndex
  else
    suiComboBox1.ItemIndex := index;
end;

end.
