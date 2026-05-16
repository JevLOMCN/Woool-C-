program NpcPageEditor;

uses
  Forms,
  clMain in 'clMain.pas' {frmMain},
  define in 'define.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TfrmMain, frmMain);
  Application.Run;
end.
