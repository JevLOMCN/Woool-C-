unit frmNewObject;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, SUIForm, StdCtrls, SUIComboBox, SUIEdit,
  SUIImagePanel, SUIGroupBox, SUIButton;

type
  TfrmNew = class(TForm)
    suiForm1: TsuiForm;
    suiGroupBox1: TsuiGroupBox;
    ie_edName: TsuiEdit;
    ie_cbTemplate: TsuiComboBox;
    Label3: TLabel;
    Label4: TLabel;
    suiButton1: TsuiButton;
    suiButton2: TsuiButton;
    procedure suiButton1Click(Sender: TObject);
    procedure suiButton2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    function OpenNewDialog( name:string; slTemplate:TStringList ):integer;
    
  end;

var
  frmNew: TfrmNew;

implementation

{$R *.dfm}

procedure TfrmNew.suiButton1Click(Sender: TObject);
begin
  ModalResult := mrOk;
end;

procedure TfrmNew.suiButton2Click(Sender: TObject);
begin
  ModalResult := mrCancel;
end;
function TfrmNew.OpenNewDialog( name:string; slTemplate:TStringList ):integer;
begin
  ie_edName.Text := name;
  ie_cbTemplate.Items := slTemplate;
  Result := ShowModal;
end;
end.
