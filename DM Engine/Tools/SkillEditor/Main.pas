unit Main;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, SUIForm, StdCtrls, SUIEdit, SUIImagePanel;

type
  TfrmMain = class(TForm)
    suiForm1: TsuiForm;
    Label1: TLabel;
    suiEdit1: TsuiEdit;
    suiSpinEdit1: TsuiSpinEdit;
    Label2: TLabel;
    suiSpinEdit2: TsuiSpinEdit;
    Label3: TLabel;
    suiSpinEdit3: TsuiSpinEdit;
    Label4: TLabel;
    suiSpinEdit4: TsuiSpinEdit;
    Label5: TLabel;
    suiPanel1: TsuiPanel;
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  frmMain: TfrmMain;

implementation

{$R *.dfm}

end.
