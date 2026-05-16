unit ItemClassEd;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, SUIForm, SUIButton, SUIImagePanel, SUIGroupBox,
  StdCtrls, SUIListBox, SUIComboBox, SUIEdit, SUIMemo, define, Menus,
  SUIPopupMenu, SUIDlg, frmNewObject;

type
  TPropEditGroup = record
    PCheck:^TsuiCheckBox;
    PName:^TsuiEdit;
    PAddValue:^TsuiNumberEdit;
  end;
  TfrmItemClassEd = class(TForm)
    suiForm1: TsuiForm;
    ie_lbItemClass: TsuiListBox;
    ie_grpPropSetting: TsuiGroupBox;
    suiButton1: TsuiButton;
    suiButton2: TsuiButton;
    suiButton3: TsuiButton;
    ie_edSp: TsuiEdit;
    suiGroupBox2: TsuiGroupBox;
    Label1: TLabel;
    ie_edClassName: TsuiEdit;
    ie_sedStdMode: TsuiSpinEdit;
    Label3: TLabel;
    ie_chkSp: TsuiCheckBox;
    ie_edAc1: TsuiEdit;
    ie_chkAC1: TsuiCheckBox;
    ie_edAc2: TsuiEdit;
    ie_chkAc2: TsuiCheckBox;
    ie_edMac1: TsuiEdit;
    ie_chkMac1: TsuiCheckBox;
    ie_edMac2: TsuiEdit;
    ie_chkMac2: TsuiCheckBox;
    ie_edDc1: TsuiEdit;
    ie_chkDc1: TsuiCheckBox;
    ie_edDc2: TsuiEdit;
    ie_chkDc2: TsuiCheckBox;
    ie_edMc1: TsuiEdit;
    ie_chkMc1: TsuiCheckBox;
    ie_edMc2: TsuiEdit;
    ie_chkMc2: TsuiCheckBox;
    ie_edSc1: TsuiEdit;
    ie_chkSc1: TsuiCheckBox;
    ie_edSc2: TsuiEdit;
    ie_chkSc2: TsuiCheckBox;
    ie_chkDura: TsuiCheckBox;
    ie_edDura: TsuiEdit;
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
    Label2: TLabel;
    ie_memoShapes: TsuiMemo;
    ie_cbType: TsuiComboBox;
    Label4: TLabel;
    Label5: TLabel;
    mEditor: TsuiPopupMenu;
    N1: TMenuItem;
    N2: TMenuItem;
    procedure suiButton3Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure N1Click(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure ie_lbItemClassMouseDown(Sender: TObject;
      Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
  private
    { Private declarations }
    mClass:TItemClass;
    PropEditGroups:array[0..11] of TPropEditGroup;
  public
    procedure SetGuiData(ItemClass: TItemClass);
    { Public declarations }
  end;

var
  frmItemClassEd: TfrmItemClassEd;

implementation

{$R *.dfm}

procedure TfrmItemClassEd.suiButton3Click(Sender: TObject);
var
  i:integer;
  s:string;
begin
  mClass := TItemClass.Create;
  mClass.ParseShapes( TStringList( ie_memoShapes.Lines ) );
  for i := 0 to mClass.Shapes.Count -1 do
  begin
    s := mClass.Shapes[i] + '=' + IntToStr( integer( mClass.Shapes.Objects[i] ) ) + '|';
    ie_lbItemClass.AddItem( s, nil );
  end;
  SetGuiData( mClass );
end;

procedure TfrmItemClassEd.FormCreate(Sender: TObject);
begin
  PropEditGroups[0].PCheck := @ie_chkSp;
  PropEditGroups[0].PName := @ie_edSp;
  PropEditGroups[0].PAddValue := @ie_sedSp;
  PropEditGroups[1].PCheck := @ie_chkAc1;
  PropEditGroups[1].PName := @ie_edAc1;
  PropEditGroups[1].PAddValue := @ie_sedAc1;
  PropEditGroups[2].PCheck := @ie_chkAc2;
  PropEditGroups[2].PName := @ie_edAc2;
  PropEditGroups[2].PAddValue := @ie_sedAc2;
  PropEditGroups[3].PCheck := @ie_chkMac1;
  PropEditGroups[3].PName := @ie_edMac1;
  PropEditGroups[3].PAddValue := @ie_sedMac1;
  PropEditGroups[4].PCheck := @ie_chkMac2;
  PropEditGroups[4].PName := @ie_edMac2;
  PropEditGroups[4].PAddValue := @ie_sedMac2;
  PropEditGroups[5].PCheck := @ie_chkDc1;
  PropEditGroups[5].PName := @ie_edDc1;
  PropEditGroups[5].PAddValue := @ie_sedDc1;
  PropEditGroups[6].PCheck := @ie_chkDc2;
  PropEditGroups[6].PName := @ie_edDc2;
  PropEditGroups[6].PAddValue := @ie_sedDc2;
  PropEditGroups[7].PCheck := @ie_chkMc1;
  PropEditGroups[7].PName := @ie_edMc1;
  PropEditGroups[7].PAddValue := @ie_sedMc1;
  PropEditGroups[8].PCheck := @ie_chkMc2;
  PropEditGroups[8].PName := @ie_edMc2;
  PropEditGroups[8].PAddValue := @ie_sedMc2;
  PropEditGroups[9].PCheck := @ie_chkSc1;
  PropEditGroups[9].PName := @ie_edSc1;
  PropEditGroups[9].PAddValue := @ie_sedSc1;
  PropEditGroups[10].PCheck := @ie_chkSc2;
  PropEditGroups[10].PName := @ie_edSc2;
  PropEditGroups[10].PAddValue := @ie_sedSc2;
  PropEditGroups[11].PCheck := @ie_chkDura;
  PropEditGroups[11].PName := @ie_edDura;
  PropEditGroups[11].PAddValue := @ie_sedDura;
end;

procedure TfrmItemClassEd.SetGuiData(ItemClass: TItemClass);
var
  i:integer;
  s:string;
begin
  ie_edClassName.Text := ItemClass.Name;
  ie_sedStdMode.Value := ItemClass.StdMode;
  ie_cbType.ItemIndex := ItemClass.DataType;
  for i := 0 to ItemClass.Shapes.Count - 1 do
  begin
    s := ItemClass.Shapes[i] + ' = ' + IntToStr( Integer( ItemClass.Shapes.Objects[i] ) );
    ie_memoShapes.Lines.Add(s);
  end;
  for i := 0 to 11 do
  begin
    if itemClass.Props[i].used then
    begin
      PropEditGroups[i].PCheck.Checked := TRUE;
      PropEditGroups[i].PName.Enabled := TRUE;
      PropEditGroups[i].PName.Text := ItemClass.Props[i].name;
      PropEditGroups[i].PAddValue.Enabled := TRUE;
      PropEditGroups[i].PAddValue.Value := ItemClass.Props[i].offsetvalue;
    end
    else
    begin
      PropEditGroups[i].PCheck.Checked := FALSE;
      PropEditGroups[i].PName.Text := '';
      PropEditGroups[i].PName.Enabled := FALSE;
      PropEditGroups[i].PAddValue.Enabled := FALSE;
      PropEditGroups[i].PAddValue.Value := 0;
    end;
  end;
end;

procedure TfrmItemClassEd.N1Click(Sender: TObject);
var
  iclass:TItemClass;
begin
  if frmNew.OpenNewDialog( '', TStringList(ie_lbItemClass.Items) ) = mrOk then
  begin
    if gItemClasses.IndexOf( frmNew.ie_edName.Text ) = -1 then
    begin
      iclass := TItemClass.Create;
      if iClass <> nil then
      begin
        gItemClasses.AddObject( frmNew.ie_edName.Text, iClass );
        ie_lbItemClass.Items := gItemClasses;
        ie_lbItemClass.ItemIndex := ie_lbItemClass.Items.IndexOf(frmNew.ie_edName.Text);
      end;
    end
    else
    begin
      exit;
    end;
  end;
end;

procedure TfrmItemClassEd.FormShow(Sender: TObject);
begin
  ie_lbItemClass.Clear;
  ie_lbItemClass.Items := gItemClasses;
end;

procedure TfrmItemClassEd.ie_lbItemClassMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  iClass:TItemClass;
begin
  if Button = mbLeft then
  begin
    if ie_lbItemClass.ItemIndex <> -1 then
    begin
      iClass := TItemClass(gItemClasses.Objects[ie_lbItemClass.ItemIndex]);
      SetGuiData( iClass );
    end;
  end;
end;

end.
