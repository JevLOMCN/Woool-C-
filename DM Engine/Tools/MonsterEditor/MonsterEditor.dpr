program MonsterEditor;

uses
  Forms,
  frmMonsterEdit in 'frmMonsterEdit.pas' {_frmMonsterEdit},
  define in 'define.pas',
  newClass in 'newClass.pas' {frmNewClass};

{$R *.res}

begin
  Application.Initialize;
  Application.Title := '¹ÖÎï±à¼­Æ÷';
  Application.CreateForm(T_frmMonsterEdit, _frmMonsterEdit);
  Application.CreateForm(TfrmNewClass, frmNewClass);
  Application.Run;
end.
