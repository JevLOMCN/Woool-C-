program ItemEditor;

uses
  Forms,
  frmItemEditor in 'frmItemEditor.pas' {frmMainEditor},
  ItemClassEd in 'ItemClassEd.pas' {frmItemClassEd},
  SglFile in 'SglFile.pas',
  define in 'define.pas',
  frmNewObject in 'frmNewObject.pas' {frmNew};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TfrmMainEditor, frmMainEditor);
  Application.CreateForm(TfrmItemClassEd, frmItemClassEd);
  Application.CreateForm(TfrmNew, frmNew);
  Init;
  Application.Run;
end.
