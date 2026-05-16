program MonGenEditor;

uses
  Forms,
  editor in 'editor.pas' {frmGenEditor},
  WooolMap in 'WooolMap.pas',
  mapdesc in 'mapdesc.pas',
  define in 'define.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TfrmGenEditor, frmGenEditor);
  Application.Run;
end.
