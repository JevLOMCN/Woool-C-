unit frmMonsterEdit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, SUIMgr, ExtCtrls, SUIForm, StdCtrls, SUIListBox, SUIImagePanel,
  SUIGroupBox, SUIComboBox, SUIEdit, CheckLst, SUICheckListBox,
  SUIScrollBar, SUIButton, ComCtrls, SUITreeView, SUIListView, SUIURLLabel,
  SUIDlg, Menus, SUIPopupMenu, SUIColorBox, define, SUIMainMenu,
  SUIPageControl, SUITabControl, newClass;

type
  TPClb = ^TsuiCheckListBox;
  T_frmMonsterEdit = class(TForm)
    frmMonsterEdit: TsuiForm;
    suiThemeManager1: TsuiThemeManager;
    suiBuiltInFileTheme1: TsuiBuiltInFileTheme;
    me_gbDropItem: TsuiGroupBox;
    suiListView1: TsuiListView;
    Label35: TLabel;
    suiEdit10: TsuiEdit;
    Label36: TLabel;
    suiSpinEdit22: TsuiSpinEdit;
    Label37: TLabel;
    suiSpinEdit23: TsuiSpinEdit;
    Label38: TLabel;
    me_idNewClass: TsuiInputDialog;
    me_ListPopmenu: TsuiPopupMenu;
    me_MenuInsert: TMenuItem;
    me_MenuAdd: TMenuItem;
    me_MenuDelete: TMenuItem;
    me_lbClassList: TsuiListBox;
    me_mdMain: TsuiMessageDialog;
    suiScrollBar1: TsuiScrollBar;
    me_MainMenu: TsuiMainMenu;
    me_mFile: TMenuItem;
    me_miOpen: TMenuItem;
    me_miSave: TMenuItem;
    me_miSaveAs: TMenuItem;
    N2: TMenuItem;
    me_miExit: TMenuItem;
    me_sdMain: TSaveDialog;
    me_odMain: TOpenDialog;
    me_miNew: TMenuItem;
    suiPanel9: TsuiPanel;
    me_plChange1: TsuiPanel;
    me_lbChg1I: TLabel;
    me_chkChange1: TsuiCheckBox;
    me_cbChg1S2: TsuiComboBox;
    me_cbChangeInto1: TsuiComboBox;
    me_chkChg1Anim: TsuiCheckBox;
    me_vChg1P2: TsuiSpinEdit;
    me_vChg1P1: TsuiSpinEdit;
    me_cbChg1S1: TsuiComboBox;
    me_plChange2: TsuiPanel;
    me_lbChg2I: TLabel;
    me_chkChange2: TsuiCheckBox;
    me_cbChg2S1: TsuiComboBox;
    me_cbChg2S2: TsuiComboBox;
    me_vChg2P2: TsuiSpinEdit;
    me_vChg2P1: TsuiSpinEdit;
    me_cbChangeInto2: TsuiComboBox;
    me_chkChg2Anim: TsuiCheckBox;
    me_plChange3: TsuiPanel;
    me_lbChg3I: TLabel;
    me_chkChange3: TsuiCheckBox;
    me_cbChangeInto3: TsuiComboBox;
    me_chkChg3Anim: TsuiCheckBox;
    me_vChg3P2: TsuiSpinEdit;
    me_cbChg3S2: TsuiComboBox;
    me_cbChg3S1: TsuiComboBox;
    me_vChg3P1: TsuiSpinEdit;
    me_plBase: TsuiPanel;
    me_lbRace: TLabel;
    me_lbLevel: TLabel;
    me_lbViewName: TLabel;
    me_lbClassName: TLabel;
    me_cbRace: TsuiComboBox;
    me_vLevel: TsuiSpinEdit;
    me_edViewName: TsuiEdit;
    me_edClassName: TsuiEdit;
    Label51: TLabel;
    me_lbChg1A: TLabel;
    me_lbChg2A: TLabel;
    me_lbChg3A: TLabel;
    me_plProp: TsuiPanel;
    me_lbHp: TLabel;
    me_lbMP: TLabel;
    me_lbSpeed: TLabel;
    me_lbHit: TLabel;
    me_lbExp: TLabel;
    Label2: TLabel;
    me_lbAC: TLabel;
    me_lbDC: TLabel;
    Label4: TLabel;
    Label6: TLabel;
    Label8: TLabel;
    Label40: TLabel;
    Label39: TLabel;
    me_lbMC: TLabel;
    me_lbMAC: TLabel;
    me_vHp: TsuiSpinEdit;
    me_vMp: TsuiSpinEdit;
    me_vSpeed: TsuiSpinEdit;
    me_vHit: TsuiSpinEdit;
    me_vExp: TsuiSpinEdit;
    me_vAc2: TsuiSpinEdit;
    me_vAc1: TsuiSpinEdit;
    me_vDC1: TsuiSpinEdit;
    me_vDC2: TsuiSpinEdit;
    me_vMAC2: TsuiSpinEdit;
    me_vMC2: TsuiSpinEdit;
    me_vWalkDelay: TsuiSpinEdit;
    me_vAiDelay: TsuiSpinEdit;
    me_vMC1: TsuiSpinEdit;
    me_vMAC1: TsuiSpinEdit;
    me_plPetSetting: TsuiPanel;
    Label17: TLabel;
    Label18: TLabel;
    me_cbPetType: TsuiComboBox;
    me_cbStopAt: TsuiComboBox;
    Label22: TLabel;
    me_vRecoverHp: TsuiSpinEdit;
    Label25: TLabel;
    me_vRecoverHpTime: TsuiSpinEdit;
    Label27: TLabel;
    Label31: TLabel;
    me_vRecoverMpTime: TsuiSpinEdit;
    Label29: TLabel;
    me_vRecoverMp: TsuiSpinEdit;
    Label28: TLabel;
    me_plSProp: TsuiPanel;
    Label30: TLabel;
    Label32: TLabel;
    Label33: TLabel;
    Label34: TLabel;
    me_clbSpecialFlag: TsuiCheckListBox;
    me_vCallRate: TsuiSpinEdit;
    me_vAntSoulWall: TsuiSpinEdit;
    me_vAntTrouble: TsuiSpinEdit;
    me_vAntHolyWord: TsuiSpinEdit;
    me_chkSFlagAll: TsuiCheckBox;
    me_sbVSF: TsuiScrollBar;
    me_plAISetting: TsuiPanel;
    Label23: TLabel;
    Label24: TLabel;
    Label26: TLabel;
    me_lbMoveStyle: TLabel;
    me_lbDieStyle: TLabel;
    Label3: TLabel;
    Label20: TLabel;
    me_clbTargetFlag: TsuiCheckListBox;
    me_vEscapeDistance: TsuiSpinEdit;
    me_vCoolEyes: TsuiSpinEdit;
    me_vViewDistance: TsuiSpinEdit;
    me_cbMoveStyle: TsuiComboBox;
    me_cbDieStyle: TsuiComboBox;
    me_cbTargetSelect: TsuiComboBox;
    me_plAttack: TsuiPanel;
    Label45: TLabel;
    Label46: TLabel;
    Label47: TLabel;
    Label44: TLabel;
    Label43: TLabel;
    Label48: TLabel;
    Label49: TLabel;
    Label50: TLabel;
    Label53: TLabel;
    Label52: TLabel;
    Label42: TLabel;
    me_a1_cbAttackStyle: TsuiComboBox;
    me_a1_vDelay: TsuiSpinEdit;
    me_a1_cbDamageType: TsuiComboBox;
    me_a1_cbDamageStyle: TsuiComboBox;
    me_a1_vAttackDistance: TsuiSpinEdit;
    me_a1_cbAppendEffect: TsuiComboBox;
    me_a1_vAppendRate: TsuiSpinEdit;
    me_a1_vCostHp: TsuiSpinEdit;
    me_a1_vCostMp: TsuiSpinEdit;
    me_a1_vDamageRange: TsuiSpinEdit;
    me_tvClassList: TsuiTreeView;
    me_pmTV: TsuiPopupMenu;
    me_miCreateNew: TMenuItem;
    me_miInsertNew: TMenuItem;
    me_miDeleteExist: TMenuItem;
    me_miCreateCopy: TMenuItem;
    Label1: TLabel;
    me_cbLockDir: TsuiComboBox;
    me_clbChg1Append: TsuiCheckListBox;
    me_clbChg3Append: TsuiCheckListBox;
    me_clbChg2Append: TsuiCheckListBox;
    suiGroupBox1: TsuiGroupBox;
    me_cbView: TsuiComboBox;
    me_lbOutView: TLabel;
    me_cbDress: TsuiComboBox;
    Label5: TLabel;
    Label7: TLabel;
    me_cbDressColor: TsuiComboBox;
    Label9: TLabel;
    me_cbHair: TsuiComboBox;
    Label10: TLabel;
    me_cbHairColor: TsuiComboBox;
    Label11: TLabel;
    me_cbSex: TsuiComboBox;
    Label12: TLabel;
    me_cbWeapon: TsuiComboBox;
    me_cbNameColor: TsuiColorBox;
    me_cbAttackAction: TsuiComboBox;
    Label13: TLabel;
    Label14: TLabel;
    me_vAppendTime: TsuiSpinEdit;
    procedure me_MenuAddClick(Sender: TObject);
    procedure me_MenuInsertClick(Sender: TObject);
    procedure me_MenuDeleteClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure me_lbClassListMouseDown(Sender: TObject;
      Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure me_miSaveClick(Sender: TObject);
    procedure suiButton1Click(Sender: TObject);
    procedure suiButton2Click(Sender: TObject);
    procedure suiButton3Click(Sender: TObject);
    procedure me_miNewClick(Sender: TObject);
    procedure me_miOpenClick(Sender: TObject);
    procedure me_miSaveAsClick(Sender: TObject);
    procedure me_chkSFlagAllClick(Sender: TObject);
    procedure me_chkChange1Click(Sender: TObject);
    procedure me_plChange1Pop(Sender: TObject);
    procedure me_plChange2Pop(Sender: TObject);
    procedure me_plChange3Pop(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure me_miExitClick(Sender: TObject);
    procedure me_miCreateCopyClick(Sender: TObject);
    procedure me_miCreateNewClick(Sender: TObject);
    procedure me_chkChange2Click(Sender: TObject);
    procedure me_chkChange3Click(Sender: TObject);
    procedure me_lbClassListKeyUp(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure me_lbClassListMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
  private
    { Private declarations }
    //ClassList:TList;
    CurIndex:Integer;
    strFile:String;
  public
    function InsertNewClass(sName,sTemplate: string; index: integer): Boolean;
    procedure DeleteClass(index: integer);
    procedure SaveGui(index: integer);
    procedure LoadGui(index: integer);
    procedure Save(filename: string);
    procedure Clear;
    procedure Load(filename: string);
    procedure SetGui( guiClass:TMonsterClass );
    procedure SetClassList(AComboBox: TComboBox);
    procedure PromptSave;
    procedure PromptLoad;
    procedure PromptSaveAs;
    procedure SetCurFile(sfilename: string);
    procedure SetComboBoxIndex( AComboBox:TComboBox;Index:integer );
    procedure EnableChange(index: integer;bEnable:Boolean);
    function GetClassByName(sName: string): TMonsterClass;
    function GetClbFlag( AClb:TPClb ):DWORD;
    procedure SetClbFlag( AClb:TPClb; bFlag:DWORD );
    function GetHumanView: integer;
    procedure SetHumanView(view: integer);
    procedure addlog(slog: string);
    { Public declarations }
  end;

var
  _frmMonsterEdit: T_frmMonsterEdit;

implementation

{$R *.dfm}

function T_frmMonsterEdit.InsertNewClass(sName,sTemplate: string;
  index: integer): Boolean;
var
  newClass:TMonsterClass;
  templateClass:TMonsterClass;
begin
  Result := true;
  SaveGui( CurIndex );
  if me_lbClassList.Items.IndexOf(sName) <> -1 then
  begin
    me_mdMain.Text := sName + ' 在列表中已经存在!';
    me_mdMain.ShowModal;
    Result := false;
  end
  else
  begin
    //pClass := allocmem( sizeof( TMonsterClass ) );
    newClass := TMonsterClass.Create;
    if newClass <> nil then
    begin
      if sTemplate <> '' then
      begin
        templateClass := GetClassByName( sTemplate );
        if templateClass <> nil then
          newClass.Copy( templateClass );
      end;
      newClass.Base.ClassName := sName;
      if newClass.Base.ViewName = '' then newClass.Base.ViewName := sName;

      if index > me_lbClassList.Count then index := me_lbClassList.Count;
      me_lbClassList.Items.InsertObject(index,sName,newClass);
      me_lbClassList.ItemIndex := index;
      LoadGui( index );
    end;
  end;
end;

procedure T_frmMonsterEdit.me_MenuAddClick(Sender: TObject);
begin

  SetClassList(frmNewClass.GetTemplateComboBox);

  if (frmNewClass.ShowModal = mrOk) and (frmNewClass.ValueText <> '' ) then
  begin
    //SaveGui(
    InsertNewClass( frmNewClass.ValueText, frmNewClass.TemplateText, me_lbClassList.Count );
  end;
end;

procedure T_frmMonsterEdit.me_MenuInsertClick(Sender: TObject);
{var
  index:integer;}
begin
  if me_lbClassList.ItemIndex <> -1 then
  begin
    SetClassList(frmNewClass.GetTemplateComboBox);

    if (frmNewClass.ShowModal = mrOk) and (frmNewClass.ValueText <> '' ) then
    begin
      InsertNewClass( frmNewClass.ValueText, frmNewClass.TemplateText, me_lbClassList.ItemIndex );
    end;
  end;
end;

procedure T_frmMonsterEdit.me_MenuDeleteClick(Sender: TObject);
begin
  if me_lbClassList.ItemIndex <> -1 then
  begin
    DeleteClass( me_lbClassList.ItemIndex );    
  end;
end;

procedure T_frmMonsterEdit.DeleteClass(index: integer);
var
  delClass:TMonsterClass;
begin
  AddLog( 'DeleteClass' );
  if index < me_lbClassList.Count then
  begin
    delClass := TMonsterClass(me_lbClassList.Items.Objects[index]);
    if CurIndex = index then
    begin
      delClass.Clear;
      SetGui( delClass );
    end;
    delClass.Free;
    me_lbClassList.Items.Delete(index);
    me_lbClassList.Refresh;
    if CurIndex = index then
    begin
      {delClass.Clear;
      SetGui( delClass );}
      CurIndex := -1;
      if index >= me_lbClassList.Items.Count then
        index := me_lbClassList.Items.Count -1;
        me_lbClassList.ItemIndex := index;
      LoadGui( index );
    end;
  end;
end;
procedure T_frmMonsterEdit.SetComboBoxIndex( AComboBox:TComboBox;Index:integer );
begin
  if (index < 0) or (index >= AComboBox.Items.Count) then exit;
  AComboBox.ItemIndex := index; 
end;
procedure T_frmMonsterEdit.FormCreate(Sender: TObject);
var
  slSetting:TStringList;
  slTarget:TStrings;
  slList:TStringList;
  i,index:integer;
  p:pchar;
  stemp:string;
begin
//  ClassList := TList.Create;
  slList := TStringList.Create;
  slSetting := TStringList.Create;
  slList.AddObject( 'race', me_cbRace.Items );
  slList.AddObject( 'view', me_cbView.Items );
  slList.AddObject( 'pettype', me_cbPetType.Items );
  slList.AddObject( 'movestyle', me_cbMoveStyle.Items );
  slList.AddObject( 'diestyle', me_cbDieStyle.Items );
  slList.AddObject( 'targetselect', me_cbTargetSelect.Items );
  slList.AddObject( 'targettype', me_clbTargetFlag.Items );
  slList.AddObject( 'attackstyle', me_a1_cbAttackStyle.Items );
  slList.AddObject( 'damagestyle', me_a1_cbDamageStyle.Items );
  slList.AddObject( 'damagetype', me_a1_cbDamageType.Items );
  slList.AddObject( 'attackappend', me_a1_cbAppendEffect.Items );
  slList.AddObject( 'specialflag', me_clbSpecialFlag.Items );
  slList.AddObject( 'changesituation', me_cbChg1S1.Items );
  slList.AddObject( 'changeappend', me_clbChg1Append.Items );
  slList.AddObject( 'dressview', me_cbDress.Items );
  slList.AddObject( 'dresscolor', me_cbDressColor.Items );
  slList.AddObject( 'hairview', me_cbHair.Items );
  slList.AddObject( 'haircolor', me_cbHairColor.Items );
  slList.AddObject( 'weaponview', me_cbWeapon.Items );
  slTarget := nil;
  if slSetting <> nil then
  begin
    slSetting.LoadFromFile( 'Setting.txt' );
  end;
  for i := 0 to slSetting.Count - 1 do
  begin
    p := pchar(slSetting.Strings[i]);
    if p[0] = '@' then
    begin
      stemp := p + 1;
      index := slList.IndexOf( stemp );
      if index <> -1 then
        slTarget := TStrings(slList.Objects[index])
      else
        slTarget := nil;
      if slTarget <> nil then slTarget.Clear;
    end
    else
    begin
      stemp := trim( p );
      if stemp <> '' then
      begin
        if slTarget <> nil then slTarget.Add(stemp);
      end;
    end;
  end;
  me_clbChg2Append.Items := me_clbChg1Append.Items;
  me_clbChg3Append.Items := me_clbChg1Append.Items;
  me_cbChg1S2.Items := me_cbChg1S1.Items;
  me_cbChg2S1.Items := me_cbChg1S1.Items;
  me_cbChg2S2.Items := me_cbChg1S1.Items;
  me_cbChg3S1.Items := me_cbChg1S1.Items;
  me_cbChg3S2.Items := me_cbChg1S1.Items;

  SetComboBoxIndex(TComboBox(me_cbRace),0 );
  SetComboBoxIndex(TComboBox(me_cbView),0 );
  SetComboBoxIndex(TComboBox(me_cbPetType),0 );
  SetComboBoxIndex(TComboBox(me_cbMoveStyle),0 );
  SetComboBoxIndex(TComboBox(me_cbDieStyle),0 );
  SetComboBoxIndex(TComboBox(me_cbTargetSelect),0 );
  SetComboBoxIndex(TComboBox(me_a1_cbAttackStyle),0 );
  SetComboBoxIndex(TComboBox(me_a1_cbDamageStyle),0 );
  SetComboBoxIndex(TComboBox(me_a1_cbDamageType),0 );
  SetComboBoxIndex(TComboBox(me_a1_cbAppendEffect),0 );
  SetComboBoxIndex(TComboBox(me_cbChg1S1),0 );
  SetComboBoxIndex(TComboBox(me_cbChg1S2),0 );
  SetComboBoxIndex(TComboBox(me_cbChg2S1),0 );
  SetComboBoxIndex(TComboBox(me_cbChg2S2),0 );
  SetComboBoxIndex(TComboBox(me_cbChg3S1),0 );
  SetComboBoxIndex(TComboBox(me_cbChg3S2),0 );
  Fill256ColorToComboBox( TComboBox(me_cbNameColor) );
  me_cbNameColor.ItemIndex := 0;
  //SetComboBoxIndex(TComboBox(me_cbChg1Append),0 );
  //SetComboBoxIndex(TComboBox(me_cbChg2Append),0 );
  //SetComboBoxIndex(TComboBox(me_cbChg3Append),0 );
  Clear;
end;

procedure T_frmMonsterEdit.SaveGui(index: integer);
var
  saveClass:TMonsterClass;
  //i:integer;
begin
  AddLog( 'SaveGui(' + IntToStr( index ) + ')' );
  if (index < 0 ) or (index >= me_lbClassList.Count) then exit;
  saveClass := TMonsterClass(me_lbClassList.Items.Objects[index]);
  if saveClass = nil then exit;
  if me_edClassName.Text <> me_lbClassList.Items.Strings[index] then
  begin
    if me_lbClassList.Items.IndexOf(me_edClassName.Text) <> -1 then
      me_edClassName.Text := me_lbClassList.Items.Strings[index]
    else
      me_lbClassList.Items.Strings[index] := me_edClassName.Text;
  end;

  //  Base
  saveClass.Base.ClassName := me_edClassName.Text;
  saveClass.Base.ViewName := me_edViewName.Text;
  saveClass.Base.OutView := me_cbView.ItemIndex;
  if saveClass.Base.OutView = 0 then
    saveClass.Base.OutView_h := GetHumanView
  else
    saveClass.Base.OutView_h := 0;
  saveClass.Base.Level := me_vLevel.Value;
  saveClass.Base.Race := me_cbRace.ItemIndex;
  saveClass.Base.NameColor := me_cbNameColor.ItemIndex;  //me_vNameColor.Value;
  //  Prop
  saveClass.Prop.Hp := me_vHp.Value;
  saveClass.Prop.Mp := me_vMp.Value;
  saveClass.Prop.Hit := me_vHit.Value;
  saveClass.Prop.Speed := me_vSpeed.Value;
  saveClass.Prop.Ac1 := me_vAc1.Value;
  saveClass.Prop.Ac2 := me_vAc2.Value;
  saveClass.Prop.Dc1 := me_vDc1.Value;
  saveClass.Prop.Dc2 := me_vDc2.Value;
  saveClass.Prop.Mc1 := me_vMc1.Value;
  saveClass.Prop.Mc2 := me_vMc2.Value;
  saveClass.Prop.Mac1 := me_vMac1.Value;
  saveClass.Prop.Mac2 := me_vMac2.Value;
  saveClass.Prop.AiDelay := me_vAiDelay.Value;
  saveClass.Prop.WalkDelay := me_vWalkDelay.Value;
  saveClass.Prop.Exp := me_vExp.Value;
  saveClass.Prop.RecoverHp := me_vRecoverHp.Value;
  saveClass.Prop.RecoverHpTime := me_vRecoverHpTime.Value;
  saveClass.Prop.RecoverMp := me_vRecoverMp.Value;
  saveClass.Prop.RecoverMpTime := me_vRecoverMpTime.Value;

  //  Special flag
  saveClass.SFlag.pFlag := GetClbFlag( @me_clbSpecialFlag);
  {for i := 0 to me_clbSpecialFlag.Count -1 do
  begin
    if me_clbSpecialFlag.Checked[i] then
      saveClass.SFlag.pFlag := saveClass.SFlag.pFlag or (1 shl i );
  end;}
  saveClass.SFlag.CallRate := me_vCallRate.Value;
  saveClass.SFlag.AntSoulWall := me_vAntSoulWall.Value;
  saveClass.SFlag.AntTrouble := me_vAntTrouble.Value;
  saveClass.SFlag.AntHolyWord := me_vAntHolyWord.Value;

  //  Ai Setting
  saveClass.AiSetting.MoveStyle := me_cbMoveStyle.ItemIndex;
  saveClass.AiSetting.DieStyle := me_cbDieStyle.ItemIndex;
  saveClass.AiSetting.TargetSelect := me_cbTargetSelect.ItemIndex;
  saveClass.AiSetting.TargetFlag := GetClbFlag( @me_clbTargetFlag);
  {for i := 0 to me_clbTargetFlag.Count -1 do
  begin
    if me_clbTargetFlag.Checked[i] then
      saveClass.AiSetting.TargetFlag := saveClass.AiSetting.TargetFlag or (1 shl i);
  end;}
  saveClass.AiSetting.ViewDistance := me_vViewDistance.Value;
  saveClass.AiSetting.CoolEyes := me_vCoolEyes.Value;
  saveClass.AiSetting.EscapeDistance := me_vEscapeDistance.Value;
  saveClass.AiSetting.LockDirection := me_cbLockDir.ItemIndex;

  with saveClass do
  begin
    {AttackDesc[0].Situation1.Situation := me_a1_cbSituation1.ItemIndex;
    AttackDesc[0].Situation1.Param := me_a1_vSituationParam1.Value;
    AttackDesc[0].Situation2.Situation := me_a1_cbSituation2.ItemIndex;
    AttackDesc[0].Situation2.Param := me_a1_vSituationParam2.Value;}
    AttackDesc.AttackStyle := me_a1_cbAttackStyle.ItemIndex;
    AttackDesc.DamageStyle := me_a1_cbDamageStyle.ItemIndex;
    AttackDesc.DamageType := me_a1_cbDamageType.ItemIndex;
    AttackDesc.AppendEffect := me_a1_cbAppendEffect.ItemIndex;
    AttackDesc.AppendRate := me_a1_vAppendRate.Value;
    AttackDesc.AttackDistance := me_a1_vAttackDistance.Value;
    AttackDesc.DamageRange := me_a1_vDamageRange.Value;
    AttackDesc.Delay := me_a1_vDelay.Value;
    AttackDesc.CostHp := me_a1_vCostHp.Value;
    AttackDesc.CostMp := me_a1_vCostMp.Value;
    AttackDesc.Action := me_cbAttackAction.ItemIndex;
    AttackDesc.AppendTime := me_vAppendTime.Value;
  end;

  //  PetSetting
  with saveClass do
  begin
    PetSetting.pType := me_cbPetType.ItemIndex;
    PetSetting.StopAt := me_cbStopAt.ItemIndex;
  end;

  with saveClass do
  begin
    ChangeInto[0].bEnabled := me_chkChange1.Checked;
    if me_chkChange1.Checked then
    begin
      ChangeInto[0].bShowAnim := me_chkChg1Anim.Checked;
      ChangeInto[0].ChangeIntoMonster := me_cbChangeInto1.Text;
      ChangeInto[0].Situation1.Situation := me_cbChg1S1.ItemIndex;
      ChangeInto[0].Situation2.Situation := me_cbChg1S2.ItemIndex;
      ChangeInto[0].Situation1.Param := me_vChg1P1.Value;
      ChangeInto[0].Situation2.Param := me_vChg1P2.Value;
      ChangeInto[0].AppendEffect := GetClbFlag(@me_clbChg1Append);
      if ChangeInto[0].ChangeIntoMonster = '' then ChangeInto[0].bEnabled := false;
    end;
    ChangeInto[1].bEnabled := me_chkChange2.Checked;
    if me_chkChange2.Checked then
    begin
      ChangeInto[1].bShowAnim := me_chkChg2Anim.Checked;
      ChangeInto[1].ChangeIntoMonster := me_cbChangeInto2.Text;
      ChangeInto[1].Situation1.Situation := me_cbChg2S1.ItemIndex;
      ChangeInto[1].Situation2.Situation := me_cbChg2S2.ItemIndex;
      ChangeInto[1].Situation1.Param := me_vChg2P1.Value;
      ChangeInto[1].Situation2.Param := me_vChg2P2.Value;
      ChangeInto[1].AppendEffect := GetClbFlag(@me_clbChg2Append);
      if ChangeInto[1].ChangeIntoMonster = '' then ChangeInto[1].bEnabled := false;
    end;
    ChangeInto[2].bEnabled := me_chkChange3.Checked;
    if me_chkChange3.Checked then
    begin
      ChangeInto[2].bShowAnim := me_chkChg3Anim.Checked;
      ChangeInto[2].ChangeIntoMonster := me_cbChangeInto3.Text;
      ChangeInto[2].Situation1.Situation := me_cbChg3S1.ItemIndex;
      ChangeInto[2].Situation2.Situation := me_cbChg3S2.ItemIndex;
      ChangeInto[2].Situation1.Param := me_vChg3P1.Value;
      ChangeInto[2].Situation2.Param := me_vChg3P2.Value;
      ChangeInto[2].AppendEffect := GetClbFlag(@me_clbChg3Append);
      if ChangeInto[2].ChangeIntoMonster = '' then ChangeInto[2].bEnabled := false;
    end;

  end;
  //  items ( not imp )
end;

procedure T_frmMonsterEdit.LoadGui(index:integer);
var
  //i:integer;
  guiClass:TMonsterClass;
begin
  AddLog( 'LoadGui(' + IntToStr(index) + ')' );
  if CurIndex = index then exit;
  if CurIndex >= 0 then SaveGui( CurIndex );
  if (index<0) or (index>=me_lbClassList.Count ) then exit;
  CurIndex := index;
  guiClass := TMonsterClass( me_lbClassList.Items.Objects[index]);
  SetGui( guiClass );
end;

procedure T_frmMonsterEdit.me_lbClassListMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  index:integer;
  point:TPoint;
begin
  if Button = mbLeft then
  begin
    point.X := X;
    point.Y := Y;
    index := me_lbClassList.ItemAtPos( point, true );
    if index <> -1 then
    begin
      LoadGui( index );
    end;
  end;
end;

procedure T_frmMonsterEdit.Save(filename: string);
var
  //classfile:TFileStream;
  s:string;
//  pWrite:pchar;
  i,j:integer;
  writeClass:TMonsterClass;
  strlist:TStringList;
  //inifile:TIniFile;
begin
//  classfile := TFileStream.Create(filename, fmOpenWrite or fmCreate );
//  if classfile <> nil then
//  begin
    SaveGui( curindex );
    strlist := TStringList.Create;
    SetCurFile( filename );
    for i := 0 to me_lbClassList.Count -1 do
    begin
      writeClass := TMonsterClass(me_lbClassList.Items.Objects[i] );
      with writeClass do
      begin
        s := '@' + writeClass.Base.ClassName;
        strlist.Add(s);
        if Base.ViewName = '' then Base.ViewName := '<e>';
        s := 'base:' + Base.ViewName + '/' + IntToStr( Base.Race ) + '/' +
              IntToStr( Base.OutView ) + '/' + IntToStr( Base.Level ) + '/' +
              IntToStr( Base.NameColor ) + '/' + IntToStr( Base.Raceimg );
        if Base.ViewName = '<e>' then Base.ViewName := '';
        strlist.Add(s);
        s := 'prop:' + IntToStr( Prop.Hp ) + '/' + IntToStr( Prop.Mp ) + '/' +
              IntToStr( Prop.Hit ) + '/' + IntToStr( Prop.Speed ) + '/' +
              IntToStr( Prop.Ac1 ) + '/' + IntToStr( Prop.Ac2 ) + '/' +
              IntToStr( Prop.Dc1 ) + '/' + IntToStr( Prop.Dc2 ) + '/' +
              IntToStr( Prop.Mac1 ) + '/' + IntToStr( Prop.Mac2 ) + '/' +
              IntToStr( Prop.Mc1 ) + '/' + IntToStr( Prop.Mc2 ) + '/' +
              IntToStr( Prop.Exp ) + '/' + IntToStr( Prop.AiDelay ) + '/' +
              IntToStr( Prop.WalkDelay ) + '/' + IntToStr( Prop.RecoverHp ) + '/' +
              IntToStr( Prop.RecoverHpTime ) + '/' +
              IntToStr( Prop.RecoverMp ) + '/' + IntToStr( Prop.RecoverMpTime );
        strlist.Add(s);
        s := 'sprop:' + IntToStr( sFlag.pFlag ) + '/' + IntToStr( sFlag.CallRate ) + '/' +
            IntToStr( sFlag.AntSoulWall ) + '/' + IntToStr( sFlag.AntTrouble ) + '/' +
            IntToStr( sFlag.AntHolyWord );
            if sFlag.MonsterName <> '' then s := s + '/' + sFlag.MonsterName;
        strlist.Add(s);
        s := 'aiset:' + IntToStr( AiSetting.MoveStyle ) + '/' + IntToStr( AiSetting.DieStyle ) + '/' +
            IntToStr( AiSetting.TargetSelect ) + '/' + IntToStr( AiSetting.TargetFlag ) + '/' +
            IntToStr( AiSetting.ViewDistance ) + '/' + IntToStr( AiSetting.CoolEyes ) + '/' +
            IntToStr( AiSetting.EscapeDistance ) + '/' + IntToStr( AiSetting.LockDirection );
        strlist.Add(s);
        with AttackDesc do
        begin
          s := 'attack:' +
          //IntToStr( Situation1.Situation ) + '/' + IntToStr( Situation1.Param ) + '/' +
          //IntToStr( Situation2.Situation ) + '/' + IntToStr( Situation2.Param ) + '/' +
          IntToStr( AttackStyle ) + '/' +
          IntToStr( AttackDistance ) + '/' + IntToStr( Delay ) + '/' +
          IntToStr( DamageStyle ) + '/' + IntToStr( DamageRange ) + '/' +
          IntToStr( DamageType ) + '/' + IntToStr( AppendEffect ) + '/' +
          IntToStr( AppendRate ) + '/' + IntToStr( CostHp ) + '/' + IntToStr( CostMp );
        end;
        strlist.Add(s);

        s := 'petset:' + IntToStr( PetSetting.pType ) + '/' + IntToStr( PetSetting.StopAt );
        //s := s + #$0d + #$0a;
        strlist.Add(s);
        s := 'append:' + IntToStr( Base.OutView_h ) + '/' + IntToStr( AttackDesc.Action ) + '/' +
        IntToStr( AttackDesc.AppendTime ) + '/' + IntToStr( AttackDesc.AppendType );
        strlist.Add(s);
        
        if ChangeInto[0].bEnabled then
        begin
          if ChangeInto[0].ChangeIntoMonster = '' then
            ChangeInto[0].bEnabled := false
          else
          begin
            if ChangeInto[0].bShowAnim then
              j := 1
            else
              j := 0;
            s := 'chg1:' + IntToStr( ChangeInto[0].Situation1.Situation ) + '/' +
            IntToStr( ChangeInto[0].Situation1.Param ) + '/' +
            IntToStr( ChangeInto[0].Situation2.Situation ) + '/' +
            IntToStr( ChangeInto[0].Situation2.Param ) + '/' +
            ChangeInto[0].ChangeIntoMonster + '/' + IntToStr( ChangeInto[0].AppendEffect ) + '/' +
            IntToStr( j );
            strlist.Add(s);
          end;
        end;
        if ChangeInto[1].bEnabled then
        begin
          if ChangeInto[1].ChangeIntoMonster = '' then
            ChangeInto[1].bEnabled := false
          else
          begin
            if ChangeInto[1].bShowAnim then
              j := 1
            else
              j := 0;
            s := 'chg2:' + IntToStr( ChangeInto[1].Situation1.Situation ) + '/' +
            IntToStr( ChangeInto[1].Situation1.Param ) + '/' +
            IntToStr( ChangeInto[1].Situation2.Situation ) + '/' +
            IntToStr( ChangeInto[1].Situation2.Param ) + '/' +
            ChangeInto[1].ChangeIntoMonster + '/' + IntToStr( ChangeInto[1].AppendEffect ) + '/' +
            IntToStr( j );
            strlist.Add(s);
          end;
        end;
        if ChangeInto[2].bEnabled then
        begin
          if ChangeInto[2].ChangeIntoMonster = '' then
            ChangeInto[2].bEnabled := false
          else
          begin
            if ChangeInto[2].bShowAnim then
              j := 1
            else
              j := 0;
            s := 'chg3:' + IntToStr( ChangeInto[2].Situation1.Situation ) + '/' +
            IntToStr( ChangeInto[2].Situation1.Param ) + '/' +
            IntToStr( ChangeInto[2].Situation2.Situation ) + '/' +
            IntToStr( ChangeInto[2].Situation2.Param ) + '/' +
            ChangeInto[2].ChangeIntoMonster + '/' + IntToStr( ChangeInto[2].AppendEffect ) + '/' +
            IntToStr( j );
            strlist.Add(s);
          end;
        end;
      end;

    end;
    strlist.SaveToFile( filename );
    strlist.Free;
//    classfile.Free;
//  end;
end;

procedure T_frmMonsterEdit.me_miSaveClick(Sender: TObject);
begin
  PromptSave;
end;

procedure T_frmMonsterEdit.Clear;
var
  i:integer;
  clrClass:TMonsterClass;
begin
  AddLog( 'Clear' );
  for i := 0 to me_lbClassList.Count - 1 do
  begin
    clrClass := TMonsterClass(me_lbClassList.Items.Objects[i]);
    clrClass.Free;
    //clrClass := nil;
  end;
  me_lbClassList.Clear;
  CurIndex := -1;
  clrClass := TMonsterClass.Create;
  SetGui(clrClass);
  clrClass.Free;
  SetCurFile('');
end;

procedure T_frmMonsterEdit.suiButton1Click(Sender: TObject);
begin
  AddLog( 'suiButton1Click' );
  Clear;
end;

procedure T_frmMonsterEdit.suiButton2Click(Sender: TObject);
begin
  if me_odMain.Execute then
  begin
    if me_odMain.FileName <> '' then
      Load( me_odMain.FileName );
  end;
end;

procedure T_frmMonsterEdit.Load(filename: string);
var
  slFile:TStringList;
  slLine:TStringList;
  csSep:TSysCharSet;
  csWht:TSysCharSet;
  pLine:pchar;
  i,{j,}paramcount:integer;
  loadClass:TMonsterClass;
begin
  AddLog( 'Load(' + filename + ')' );
  Clear;
  csSep := ['/',':'];
  csWht := [' ', #$9];
  slFile := TStringList.Create;
  slLine := TStringList.Create;
  slFile.LoadFromFile(filename);
  loadClass := nil;
  SetCurFile( filename );
  for i := 0 to slFile.Count -1 do
  begin
    pLine := pchar( slFile.Strings[i] );
    if pLine[0] = '@' then
    begin
      if loadClass <> nil then
      begin
        if (loadClass.Base.ClassName <> '') and (loadClass.Base.ViewName <> '') then
        begin
          me_lbClassList.AddItem(loadClass.Base.ClassName, loadClass);
          loadClass := nil;
        end;
      end;
      if loadClass = nil then
        loadClass := TMonsterClass.Create
      else
        loadClass.Clear;
      loadClass.Base.ClassName := pLine+1;
    end
    else
    begin
      if loadClass = nil then continue;
      slLine.Clear;
      paramcount := ExtractStrings( csSep, csWht, pLine, slLine );
      if paramcount < 1 then continue;
      with loadClass do
      begin
        if slLine.Strings[0] = 'base' then
        begin
          if paramcount < 7 then continue;
          Base.ViewName := slLine.Strings[1];
          Base.Race := StrToIntDef( slLine.Strings[2],0 );
          Base.OutView := StrToIntDef( slLine.Strings[3],0 );
          Base.Level := StrToIntDef( slLine.Strings[4],0 );
          Base.NameColor := StrToIntDef( slLine.Strings[5],0 );
          Base.Raceimg := StrToIntDef( slLine.Strings[6],0 );
        end
        else if slLine.Strings[0] = 'prop' then
        begin
          if paramcount < 20 then continue;
          Prop.Hp := StrToIntDef( slLine.Strings[1],0 );
          Prop.Mp := StrToIntDef( slLine.Strings[2],0 );
          Prop.Hit := StrToIntDef( slLine.Strings[3],0 );
          Prop.Speed := StrToIntDef( slLine.Strings[4],0 );
          Prop.Ac1 := StrToIntDef( slLine.Strings[5],0 );
          Prop.Ac2 := StrToIntDef( slLine.Strings[6],0 );
          Prop.Dc1 := StrToIntDef( slLine.Strings[7],0 );
          Prop.Dc2 := StrToIntDef( slLine.Strings[8],0 );
          Prop.Mac1 := StrToIntDef( slLine.Strings[9],0 );
          Prop.Mac2 := StrToIntDef( slLine.Strings[10],0 );
          Prop.Mc1 := StrToIntDef( slLine.Strings[11],0 );
          Prop.Mc2 := StrToIntDef( slLine.Strings[12],0 );
          Prop.Exp := StrToIntDef( slLine.Strings[13],0 );
          Prop.AiDelay := StrToIntDef( slLine.Strings[14],0 );
          Prop.WalkDelay := StrToIntDef( slLine.Strings[15],0 );
          Prop.RecoverHp := StrToIntDef( slLine.Strings[16],0 );
          Prop.RecoverHpTime := StrToIntDef( slLine.Strings[17],0 );
          Prop.RecoverMp := StrToIntDef( slLine.Strings[18],0 );
          Prop.RecoverMpTime := StrToIntDef( slLine.Strings[19],0 );
        end
        else if slLine.Strings[0] = 'sprop' then
        begin
          if paramcount < 6 then continue;
          sFlag.pFlag := StrToIntDef( slLine.Strings[1],0 );
          sFlag.CallRate := StrToIntDef( slLine.Strings[2],0 );
          sFlag.AntSoulWall := StrToIntDef( slLine.Strings[3],0 );
          sFlag.AntTrouble := StrToIntDef( slLine.Strings[4],0 );
          sFlag.AntHolyWord := StrToIntDef( slLine.Strings[5],0 );
          if paramcount > 6 then
            sFlag.MonsterName := slLine.Strings[6];
        end
        else if slLine.Strings[0] = 'aiset' then
        begin
          if paramcount < 8 then continue;
          AiSetting.MoveStyle := StrToIntDef( slLine.Strings[1],0 );
          AiSetting.DieStyle := StrToIntDef( slLine.Strings[2],0 );
          AiSetting.TargetSelect := StrToIntDef( slLine.Strings[3],0 );
          AiSetting.TargetFlag := StrToIntDef( slLine.Strings[4],0 );
          AiSetting.ViewDistance := StrToIntDef( slLine.Strings[5],0 );
          AiSetting.CoolEyes := StrToIntDef( slLine.Strings[6],0 );
          AiSetting.EscapeDistance := StrToIntDef( slLine.Strings[7],0 );
          if paramcount > 8 then
            AiSetting.LockDirection := StrToIntDef( slLine.Strings[8], 0 );
        end
        else if slLine.Strings[0] = 'petset' then
        begin
          if paramcount < 3 then continue;
          PetSetting.pType := StrToIntDef( slLine.Strings[1],0 );
          PetSetting.StopAt := StrToIntDef( slLine.Strings[2],0 );
        end
        else if slLine.Strings[0] = 'attack' then
        begin
          if paramcount < 11 then continue;
          AttackDesc.AttackStyle := StrToIntDef( slLine.Strings[1],0 );
          AttackDesc.AttackDistance := StrToIntDef( slLine.Strings[2],0 );
          AttackDesc.Delay := StrToIntDef( slLine.Strings[3],0 );
          AttackDesc.DamageStyle := StrToIntDef( slLine.Strings[4],0 );
          AttackDesc.DamageRange := StrToIntDef( slLine.Strings[5],0 );
          AttackDesc.DamageType := StrToIntDef( slLine.Strings[6],0 );
          AttackDesc.AppendEffect := StrToIntDef( slLine.Strings[7],0 );
          AttackDesc.AppendRate := StrToIntDef( slLine.Strings[8],0 );
          AttackDesc.CostHp := StrToIntDef( slLine.Strings[9], 0 );
          AttackDesc.CostMp := StrToIntDef( slLine.Strings[10], 0 );
        end
        else if slLine.Strings[0] = 'chg1' then
        begin
          if paramcount < 8 then continue;
          ChangeInto[0].bEnabled := true;
          ChangeInto[0].Situation1.Situation := StrToIntDef( slLine.Strings[1], 0 );
          ChangeInto[0].Situation1.Param := StrToIntDef( slLine.Strings[2], 0 );
          ChangeInto[0].Situation2.Situation := StrToIntDef( slLine.Strings[3], 0 );
          ChangeInto[0].Situation2.Param := StrToIntDef( slLine.Strings[4], 0 );
          ChangeInto[0].ChangeIntoMonster := slLine.Strings[5];
          ChangeInto[0].AppendEffect := StrToIntDef( slLine.Strings[6], 0 );
          ChangeInto[0].bShowAnim := StrToBoolDef( slLine.Strings[7], false );
          if ChangeInto[0].ChangeIntoMonster = '' then
            ChangeInto[0].bEnabled := false;
        end
        else if slLine.Strings[0] = 'chg2' then
        begin
          if paramcount < 8 then continue;
          ChangeInto[1].bEnabled := true;
          ChangeInto[1].Situation1.Situation := StrToIntDef( slLine.Strings[1], 0 );
          ChangeInto[1].Situation1.Param := StrToIntDef( slLine.Strings[2], 0 );
          ChangeInto[1].Situation2.Situation := StrToIntDef( slLine.Strings[3], 0 );
          ChangeInto[1].Situation2.Param := StrToIntDef( slLine.Strings[4], 0 );
          ChangeInto[1].ChangeIntoMonster := slLine.Strings[5];
          ChangeInto[1].AppendEffect := StrToIntDef( slLine.Strings[6], 0 );
          ChangeInto[1].bShowAnim := StrToBoolDef( slLine.Strings[7], false );
          if ChangeInto[1].ChangeIntoMonster = '' then
            ChangeInto[1].bEnabled := false;
        end
        else if slLine.Strings[0] = 'chg3' then
        begin
          if paramcount < 8 then continue;
          ChangeInto[2].bEnabled := true;
          ChangeInto[2].Situation1.Situation := StrToIntDef( slLine.Strings[1], 0 );
          ChangeInto[2].Situation1.Param := StrToIntDef( slLine.Strings[2], 0 );
          ChangeInto[2].Situation2.Situation := StrToIntDef( slLine.Strings[3], 0 );
          ChangeInto[2].Situation2.Param := StrToIntDef( slLine.Strings[4], 0 );
          ChangeInto[2].ChangeIntoMonster := slLine.Strings[5];
          ChangeInto[2].AppendEffect := StrToIntDef( slLine.Strings[6], 0 );
          ChangeInto[2].bShowAnim := StrToBoolDef( slLine.Strings[7], false );
          if ChangeInto[1].ChangeIntoMonster = '' then
            ChangeInto[1].bEnabled := false;
        end
        else if slLine.Strings[0] = 'append' then
        begin
          if paramcount < 2 then continue;
          if Base.OutView = 0 then
            Base.OutView_h := StrToIntDef( slLine.Strings[1], 0 );
          if paramcount > 2 then
            AttackDesc.Action := StrToIntDef( slLine.Strings[2], 0 );
          if paramcount > 3 then
            AttackDesc.AppendTime := StrToIntDef( slLine.Strings[3], 0 );
          if paramcount > 4 then
            AttackDesc.AppendType := StrToIntDef( slLine.Strings[4], 0 );
        end
        else
        begin
        end;
      end;
    end;
  end;
  if loadClass <> nil then
  begin
    if (loadClass.Base.ClassName <> '') and (loadClass.Base.ViewName <> '') then
    begin
      me_lbClassList.AddItem(loadClass.Base.ClassName, loadClass);
      //loadClass := nil;
    end
    else
      loadClass.Free;
  end;
  slFile.Free;
  slLine.Free;
  if me_lbClassList.Count > 0 then
  begin
    me_lbClassList.ItemIndex := 0;
    LoadGui( 0 );
  end;
end;

procedure T_frmMonsterEdit.suiButton3Click(Sender: TObject);
begin
  if me_sdMain.Execute then
  begin
    if me_sdMain.FileName <> '' then
      Save( me_sdMain.FileName );
  end;
end;

procedure T_frmMonsterEdit.SetGui( guiClass:TMonsterClass );
//var
 //i:integer;
begin
  AddLog( 'SetGui flag = ' + IntTostr(guiClass.sFlag.pFlag) );
  if guiClass = nil then exit;
  {SetClassList(TComboBox(me_cbChangeInto));}
  with guiClass do
  begin
    //  Base
    me_edClassName.Text := Base.ClassName;
    me_edViewName.Text := Base.ViewName;
    me_cbRace.ItemIndex := Base.Race;
    me_cbView.ItemIndex := Base.OutView;
    if Base.OutView = 0 then
      SetHumanView( Base.OutView_h )
    else
      SetHumanView( 0 );
    me_vLevel.Value := Base.Level;
    //me_vNameColor.Value := Base.NameColor;
    me_cbNameColor.ItemIndex := Base.NameColor;
    //  Prop
    me_vHp.Value := Prop.Hp;
    me_vMp.Value := Prop.Mp;
    me_vHit.Value := Prop.Hit;
    me_vSpeed.Value := Prop.Speed;
    me_vExp.Value := Prop.Exp;
    me_vAc1.Value := Prop.Ac1;
    me_vAc2.Value := Prop.Ac2;
    me_vDc1.Value := Prop.Dc1;
    me_vDc2.Value := Prop.Dc2;
    me_vMac1.Value := Prop.Mac1;
    me_vMac2.Value := Prop.Mac2;
    me_vMc1.Value := Prop.Mc1;
    me_vMc2.Value := Prop.Mc2;
    me_vAiDelay.Value := Prop.AiDelay;
    me_vWalkDelay.Value := Prop.WalkDelay;
    me_vRecoverHp.Value := Prop.RecoverHp;
    me_vRecoverHpTime.Value := Prop.RecoverHpTime;
    me_vRecoverMp.Value := Prop.RecoverMp;
    me_vRecoverMpTime.Value := Prop.RecoverMpTime;
    //  Special Prop
    {for i := 0 to me_clbSpecialFlag.Count -1 do
    begin
      if (sFlag.pFlag and (1 shl i)) <> 0 then
        me_clbSpecialFlag.Checked[i] := true
      else
        me_clbSpecialFlag.Checked[i] := false;
    end;}
    SetClbFlag( @me_clbSpecialFlag, sFlag.pFlag );
    me_vCallRate.Value := sFlag.CallRate;
    me_vAntSoulWall.Value := sFlag.AntSoulWall;
    me_vAntTrouble.Value := sFlag.AntTrouble;
    me_vAntHolyWord.Value := sFlag.AntHolyWord;
//    me_edMonsterName.Value := sFlag.MonsterName;
    //  Ai Setting
    me_cbMoveStyle.ItemIndex := AiSetting.MoveStyle;
    me_cbDieStyle.ItemIndex := AiSetting.DieStyle;
    me_cbTargetSelect.ItemIndex := AiSetting.TargetSelect;
    {for i := 0 to me_clbTargetFlag.Count -1 do
    begin
      if (AiSetting.TargetFlag and (1 shl i)) <> 0 then
        me_clbTargetFlag.Checked[i] := true
      else
        me_clbTargetFlag.Checked[i] := false;
    end;}
    SetClbFlag( @me_clbTargetFlag, AiSetting.TargetFlag );
    me_vViewDistance.Value := AiSetting.ViewDistance;
    me_vCoolEyes.Value := AiSetting.CoolEyes;
    me_vEscapeDistance.Value := AiSetting.EscapeDistance;
    me_cbLockDir.ItemIndex := (AiSetting.LockDirection mod 8);

    me_a1_cbAttackStyle.ItemIndex := AttackDesc.AttackStyle;
    me_a1_cbDamageStyle.ItemIndex := AttackDesc.DamageStyle;
    me_a1_cbDamageType.ItemIndex := AttackDesc.DamageType;
    me_a1_vDamageRange.Value := AttackDesc.DamageRange;
    me_a1_vDelay.Value := AttackDesc.Delay;
    me_a1_cbAppendEffect.ItemIndex := AttackDesc.AppendEffect;
    me_a1_vAppendRate.Value := AttackDesc.AppendRate;
    me_a1_vAttackDistance.Value := AttackDesc.AttackDistance;
    me_a1_vCostHp.Value := AttackDesc.CostHp;
    me_a1_vCostMp.Value := AttackDesc.CostMp;
    if AttackDesc.Action >= me_cbAttackAction.Items.Count then
    begin
      me_cbAttackAction.ItemIndex := 0;
      AttackDesc.Action := 0;
    end
    else
      me_cbAttackAction.ItemIndex := AttackDesc.Action;
    me_vAppendTime.Value := AttackDesc.AppendTime;
    //  PetSetting
    me_cbPetType.ItemIndex := PetSetting.pType;
    me_cbStopAt.ItemIndex := PetSetting.StopAt;
    //  changeinto
    SetClassList( TComboBox( me_cbChangeInto1 ) );
    if ChangeInto[0].bEnabled then
    begin
      EnableChange( 0, true );
      me_cbChg1S1.ItemIndex := ChangeInto[0].Situation1.Situation;
      me_vChg1P1.Value := ChangeInto[0].Situation1.Param;
      me_cbChg1S2.ItemIndex := ChangeInto[0].Situation2.Situation;
      me_vChg1P2.Value := ChangeInto[0].Situation2.Param;
      me_cbChangeInto1.ItemIndex := me_cbChangeInto1.Items.IndexOf(ChangeInto[0].ChangeIntoMonster);
      me_chkChg1Anim.Checked := ChangeInto[0].bShowAnim;
      SetClbFlag( @me_clbChg1Append, ChangeInto[0].AppendEffect );
    end
    else
      EnableChange( 0, false );

    SetClassList( TComboBox( me_cbChangeInto2 ) );
    if ChangeInto[1].bEnabled then
    begin
      EnableChange( 1, true );
      me_cbChg2S1.ItemIndex := ChangeInto[1].Situation1.Situation;
      me_vChg2P1.Value := ChangeInto[1].Situation1.Param;
      me_cbChg2S2.ItemIndex := ChangeInto[1].Situation2.Situation;
      me_vChg2P2.Value := ChangeInto[1].Situation2.Param;
      me_cbChangeInto2.ItemIndex := me_cbChangeInto1.Items.IndexOf(ChangeInto[1].ChangeIntoMonster);
      me_chkChg2Anim.Checked := ChangeInto[1].bShowAnim;
      SetClbFlag( @me_clbChg2Append, ChangeInto[0].AppendEffect );
    end
    else
      EnableChange( 1, false );

    SetClassList( TComboBox( me_cbChangeInto3 ) );
    if ChangeInto[2].bEnabled then
    begin
      EnableChange( 2, true );
      me_cbChg3S1.ItemIndex := ChangeInto[2].Situation1.Situation;
      me_vChg3P1.Value := ChangeInto[2].Situation1.Param;
      me_cbChg3S2.ItemIndex := ChangeInto[2].Situation2.Situation;
      me_vChg3P2.Value := ChangeInto[2].Situation2.Param;
      me_cbChangeInto3.ItemIndex := me_cbChangeInto1.Items.IndexOf(ChangeInto[2].ChangeIntoMonster);
      me_chkChg3Anim.Checked := ChangeInto[2].bShowAnim;
      SetClbFlag( @me_clbChg3Append, ChangeInto[0].AppendEffect );
    end
    else
      EnableChange( 2, false );

  end;
end;

procedure T_frmMonsterEdit.SetClassList(AComboBox: TComboBox);
var
  i:integer;
begin
  AComboBox.Clear;
  for i := 0 to me_lbClassList.Count -1 do
  begin
    AComboBox.AddItem(me_lbClassList.Items.Strings[i],nil);
  end;
end;

procedure T_frmMonsterEdit.me_miNewClick(Sender: TObject);
begin
  Clear;
end;

procedure T_frmMonsterEdit.PromptSave;
begin
  if strFile = '' then
  begin
    me_sdMain.Title := '存储为..';
    if me_sdMain.Execute then
    begin
      if me_sdMain.FileName <> '' then
      begin
        Save( me_sdMain.FileName );
        //strFile := me_sdMain.FileName;
      end;
    end;
  end
  else
    Save( strFile );

end;

procedure T_frmMonsterEdit.PromptLoad;
begin
  if me_odMain.Execute then
  begin
    if me_odMain.FileName <> '' then
    begin
      Load( me_odMain.FileName );
    end;
  end;
end;

procedure T_frmMonsterEdit.PromptSaveAs;
begin
  me_sdMain.Title := '另存为..';
  if me_sdMain.Execute then
  begin
    if me_sdMain.FileName <> '' then
    begin
      Save( me_sdMain.FileName );
    end;
  end;

end;

procedure T_frmMonsterEdit.SetCurFile(sfilename: string);
begin
  strFile := sFilename;
  if strFile = '' then
    frmMonsterEdit.Caption := '怪物编辑器-未命名'
  else
    frmMonsterEdit.Caption := '怪物编辑器-' + strFile;
end;

procedure T_frmMonsterEdit.me_miOpenClick(Sender: TObject);
begin
  PromptLoad;
end;

procedure T_frmMonsterEdit.me_miSaveAsClick(Sender: TObject);
begin
  PromptSaveAs;
end;

procedure T_frmMonsterEdit.me_chkSFlagAllClick(Sender: TObject);
var
  i:integer;
begin
  AddLog( 'me_chkSFlagAllClick Sender = ' + inttostr( integer( @sender ) ) );
  if me_chkSFlagAll.Checked then
  begin
    for i := 0 to me_clbSpecialFlag.Count -1 do
    begin
      me_clbSpecialFlag.Checked[i] := true;
    end;
  end
  else
  begin
    for i := 0 to me_clbSpecialFlag.Count -1 do
    begin
      me_clbSpecialFlag.Checked[i] := false;
    end;
  end;

end;

procedure T_frmMonsterEdit.me_chkChange1Click(Sender: TObject);
begin
  EnableChange( 0, me_chkChange1.Checked );

end;

procedure T_frmMonsterEdit.me_plChange1Pop(Sender: TObject);
begin
  me_plChange2.Push;
  me_plChange3.Push;
end;

procedure T_frmMonsterEdit.me_plChange2Pop(Sender: TObject);
begin
  me_plChange1.Push;
  me_plChange3.Push;
end;

procedure T_frmMonsterEdit.me_plChange3Pop(Sender: TObject);
begin
  me_plChange1.Push;
  me_plChange2.Push;

end;

procedure T_frmMonsterEdit.FormShow(Sender: TObject);
begin
  me_plChange2.Push;
  me_plChange3.Push;
end;

procedure T_frmMonsterEdit.me_miExitClick(Sender: TObject);
begin
  Close;
end;

procedure T_frmMonsterEdit.me_miCreateCopyClick(Sender: TObject);
begin
  //
  if (me_tvClassList.Selected <> nil) and
    (me_tvClassList.Selected.Level = 0 ) then
  begin
    me_tvClassList.Items.AddChild( me_tvClassList.Selected,'abc_1' );
  end;
end;

procedure T_frmMonsterEdit.me_miCreateNewClick(Sender: TObject);
begin
  me_tvClassList.Items.AddFirst( nil, 'abc' );
end;

procedure T_frmMonsterEdit.EnableChange(index: integer;bEnable:Boolean);
begin
  AddLog( 'EnableChange' );
  if index = 0 then
  begin
    me_cbChg1S1.Enabled := bEnable;
    me_cbChg1S1.ItemIndex := 0;
    me_cbChg1S2.Enabled := bEnable;
    me_cbChg1S2.ItemIndex := 0;
    me_vChg1P1.Enabled := bEnable;
    me_vChg1P2.Enabled := bEnable;
    me_vChg1P1.Value := 0;
    me_vChg1P2.Value := 0;
    me_cbChangeInto1.Enabled := bEnable;
    me_cbChangeInto1.ItemIndex := 0;
    me_clbChg1Append.Enabled := bEnable;
    SetClbFlag( @me_clbChg1Append, 0 );
    me_chkChg1Anim.Enabled := bEnable;
    me_chkChg1Anim.Checked := false;
    me_lbChg1I.Enabled := bEnable;
    me_lbChg1A.Enabled := bEnable;
    me_chkChange1.Checked := bEnable;
  end
  else if index = 1 then
  begin
    me_cbChg2S1.Enabled := bEnable;
    me_cbChg2S1.ItemIndex := 0;
    me_cbChg2S2.Enabled := bEnable;
    me_cbChg2S2.ItemIndex := 0;
    me_vChg2P1.Enabled := bEnable;
    me_vChg2P2.Enabled := bEnable;
    me_vChg2P1.Value := 0;
    me_vChg2P2.Value := 0;
    me_cbChangeInto2.Enabled := bEnable;
    me_cbChangeInto2.ItemIndex := 0;
    me_clbChg2Append.Enabled := bEnable;
    //me_cbChg2Append.ItemIndex := 0;
    SetClbFlag( @me_clbChg2Append, 0 );
    me_chkChg2Anim.Enabled := bEnable;
    me_chkChg2Anim.Checked := false;
    me_lbChg2I.Enabled := bEnable;
    me_lbChg2A.Enabled := bEnable;
    me_chkChange2.Checked := bEnable;
  end
  else if index = 2 then
  begin
    me_cbChg3S1.Enabled := bEnable;
    me_cbChg3S1.ItemIndex := 0;
    me_cbChg3S2.Enabled := bEnable;
    me_cbChg3S2.ItemIndex := 0;
    me_vChg3P1.Enabled := bEnable;
    me_vChg3P2.Enabled := bEnable;
    me_vChg3P1.Value := 0;
    me_vChg3P2.Value := 0;
    me_cbChangeInto3.Enabled := bEnable;
    me_cbChangeInto3.ItemIndex := 0;
    me_clbChg3Append.Enabled := bEnable;
    //me_cbChg3Append.ItemIndex := 0;
    SetClbFlag( @me_clbChg3Append, 0 );
    me_chkChg3Anim.Enabled := bEnable;
    me_chkChg3Anim.Checked := false;
    me_lbChg3I.Enabled := bEnable;
    me_lbChg3A.Enabled := bEnable;
    me_chkChange3.Checked := bEnable;
  end;
end;

procedure T_frmMonsterEdit.me_chkChange2Click(Sender: TObject);
begin
  EnableChange( 1, me_chkChange2.Checked );
end;

procedure T_frmMonsterEdit.me_chkChange3Click(Sender: TObject);
begin
  EnableChange( 2, me_chkChange3.Checked );

end;

function T_frmMonsterEdit.GetClassByName(sName: string): TMonsterClass;
var
  //mc:TMonsterClass;
  idx:integer;
begin
  AddLog( 'GetClassByName(' + sName + ')' );
  idx := me_lbClassList.Items.IndexOf(sName);
  Result := nil;
  if idx >= 0 then
    Result := TMonsterClass(me_lbClassList.Items.Objects[idx]);
end;

procedure T_frmMonsterEdit.me_lbClassListKeyUp(Sender: TObject;
  var Key: Word; Shift: TShiftState);
begin
  if key = vk_Insert then
    me_MenuInsertClick( Sender )
  else if key = vk_delete then
    me_MenuDeleteClick( Sender );
  if me_lbClassList.ItemIndex <> CurIndex then
    LoadGui( me_lbClassList.ItemIndex);
end;

procedure T_frmMonsterEdit.me_lbClassListMouseUp(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  index:integer;
  point:TPoint;
begin
  if button = mbLeft then
  begin
    point.X := X;
    point.Y := Y;
    index := me_lbClassList.ItemAtPos( point, true );
    if index <> -1 then
    begin
      LoadGui( index );
    end;
  end;
end;

function T_frmMonsterEdit.GetClbFlag( AClb:TPClb ):DWORD;
var
  i:integer;
  bflag:dword;
begin

  bflag := 0;
  for i := 0 to AClb.Count -1 do
  begin
    if Aclb.Checked[i] then
      bflag := bflag or ( 1 shl i );
  end;
  Result := bFlag;
  AddLog( 'GetClbFlag = ' + IntToStr( Result ) );
end;
procedure T_frmMonsterEdit.SetClbFlag( AClb:TPClb; bFlag:DWORD );
var
  i:integer;
begin
  AddLog( 'SetClbFlag(' + '--' + IntToStr( Integer(aclb)) + '--' + IntToStr( bFlag ) + ')' );

  for i := 0 to AClb.Count -1 do
  begin
    if (bFlag and (1 shl i )) <> 0 then
      AClb.Checked[i] := true
    else
      AClb.Checked[i] := false;
  end;
end;



function T_frmMonsterEdit.GetHumanView: integer;
var
  weapon:byte;
  dress:byte;
  dresscolor:byte;
  hair:byte;
  haircolor:byte;
  sex:byte;
begin
  weapon := me_cbWeapon.ItemIndex;
  dress := me_cbDress.ItemIndex;
  dresscolor := me_cbDressColor.ItemIndex;
  hair := me_cbHair.ItemIndex;
  haircolor := me_cbHairColor.ItemIndex;
  sex := me_cbSex.ItemIndex;

  Result := ((weapon shl 8 ) or ((dresscolor and $f)shl 24 ) or
  ((dress and $f)shl 28) or ((hair and $f)shl 16 ) or
  ((haircolor and $f)shl 20) or (sex and $ff ) );
  AddLog( 'GetHumanView = ' + IntToStr( Result ) );
end;

procedure T_frmMonsterEdit.SetHumanView(view: integer);
var
  weapon:byte;
  dress:byte;
  dresscolor:byte;
  hair:byte;
  haircolor:byte;
  sex:byte;
begin
  AddLog( 'SetHumanView(' + IntToStr( view ) + ')' );
  weapon := (view and $ff00) shr 8;
  dresscolor := (view and $f000000 ) shr 24;
  dress := (view and $f0000000 ) shr 28;
  hair := (view and $f0000 ) shr 16;
  haircolor := (view and $f00000 ) shr 20;
  sex := (view and $ff );

  if sex > 0 then sex := 1;

  //me_cbView.ItemIndex := 0;
  me_cbWeapon.ItemIndex := weapon;
  me_cbDress.ItemIndex := dress;
  me_cbDressColor.ItemIndex := dresscolor;
  me_cbHair.ItemIndex := hair;
  me_cbHairColor.ItemIndex := haircolor;
  me_cbSex.ItemIndex := sex;
end;

procedure T_frmMonsterEdit.addlog(slog: string);
begin
  //listbox1.AddItem( slog, nil );
end;

end.
