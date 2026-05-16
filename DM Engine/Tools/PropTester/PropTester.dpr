program PropTester;

uses
  Forms,
  Tester in 'Tester.pas' {Form1},
  define in 'define.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
