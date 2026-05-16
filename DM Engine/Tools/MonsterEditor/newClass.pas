unit newClass;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, SUIEdit, ExtCtrls, SUIForm, SUIComboBox, SUIButton,
  SUIMgr;

type
  TfrmNewClass = class(TForm)
    suiForm1: TsuiForm;
    me_edClassName: TsuiEdit;
    Label1: TLabel;
    me_cbTemplate: TsuiComboBox;
    Label2: TLabel;
    me_btOk: TsuiButton;
    me_btCancel: TsuiButton;
    suiBuiltInFileTheme1: TsuiBuiltInFileTheme;
    suiThemeManager1: TsuiThemeManager;
    Label3: TLabel;
    procedure me_btOkClick(Sender: TObject);
    procedure me_cbTemplateChange(Sender: TObject);
  private
    { Private declarations }
  public
    ValueText:String;
    TemplateText:String;
    function GetTemplateComboBox:TComboBox;
    { Public declarations }
  end;

var
  frmNewClass: TfrmNewClass;

implementation


{$R *.dfm}
function TfrmNewClass.GetTemplateComboBox:TComboBox;
begin
  result := TComboBox(me_cbTemplate);
end;
procedure TfrmNewClass.me_btOkClick(Sender: TObject);
begin
  ValueText := me_edClassName.Text;
  TemplateText := me_cbTemplate.Text;
end;

procedure TfrmNewClass.me_cbTemplateChange(Sender: TObject);
begin
  if (me_cbTemplate.Text <> '') and (me_edClassName.Text = '') then
  begin
    me_edClassName.Text := me_cbTemplate.Text + '_';
    me_edClassName.SetFocus;
  end;
end;

end.
