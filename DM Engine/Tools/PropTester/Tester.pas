unit Tester;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, define, Gauges, StdCtrls, Spin;

type
  TForm1 = class(TForm)
    GroupBox1: TGroupBox;
    vAttackTime: TSpinEdit;
    Gauge1: TGauge;
    vC1Lucky: TSpinEdit;
    Label1: TLabel;
    v: TLabel;
    vC1Dawn: TSpinEdit;
    Label3: TLabel;
    vC1Ac1: TSpinEdit;
    Label8: TLabel;
    vC1Hitrate: TSpinEdit;
    Label9: TLabel;
    vC1Escape: TSpinEdit;
    Label10: TLabel;
    vC1MagEscape: TSpinEdit;
    vC1Ac2: TSpinEdit;
    Label4: TLabel;
    Label5: TLabel;
    vC1Mac1: TSpinEdit;
    Label6: TLabel;
    vC1Mac2: TSpinEdit;
    Label7: TLabel;
    vC1Dc1: TSpinEdit;
    Label11: TLabel;
    vC1Dc2: TSpinEdit;
    Label12: TLabel;
    vC1Mc1: TSpinEdit;
    Label13: TLabel;
    vC1Mc2: TSpinEdit;
    Label14: TLabel;
    vC1Sc1: TSpinEdit;
    Label15: TLabel;
    vC1Sc2: TSpinEdit;
    GroupBox2: TGroupBox;
    Gauge2: TGauge;
    Label16: TLabel;
    Label17: TLabel;
    Label18: TLabel;
    Label19: TLabel;
    Label20: TLabel;
    Label21: TLabel;
    Label22: TLabel;
    Label23: TLabel;
    Label24: TLabel;
    Label25: TLabel;
    Label26: TLabel;
    Label27: TLabel;
    Label28: TLabel;
    Label29: TLabel;
    Label30: TLabel;
    vC2Lucky: TSpinEdit;
    vC2Dawn: TSpinEdit;
    vC2Ac1: TSpinEdit;
    vC2Hitrate: TSpinEdit;
    vC2Escape: TSpinEdit;
    vC2MagEscape: TSpinEdit;
    vC2Ac2: TSpinEdit;
    vC2Mac1: TSpinEdit;
    vC2Mac2: TSpinEdit;
    vC2Dc1: TSpinEdit;
    vC2Dc2: TSpinEdit;
    vC2Mc1: TSpinEdit;
    vC2Mc2: TSpinEdit;
    vC2Sc1: TSpinEdit;
    vC2Sc2: TSpinEdit;
    ListBox1: TListBox;
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    Label2: TLabel;
    Button4: TButton;
    procedure FormCreate(Sender: TObject);
    procedure vC1LuckyChange(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure vC2LuckyChange(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
  private
    { Private declarations }
    //Object1,Object2:TObjectDesc;
    C1,C2:TWooolObject;
    fDisableSet:Boolean;
  public
    procedure UpdateC1FromGui;
    procedure UpdateC1ToGui;
    procedure UpdateC2FromGui;
    procedure UpdateC2ToGui;
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

procedure TForm1.FormCreate(Sender: TObject);
begin
  Randomize;
  c1 := TWooolObject.Create;
  c2 := TWooolObject.Create;
  c1.RandomValue;
  c2.RandomValue;
  UpdateC1ToGui;
  UpdateC2ToGui;
  fDisableSet := false;
end;



procedure TForm1.vC1LuckyChange(Sender: TObject);
begin
  UpdateC1FromGui;
end;

procedure TForm1.Button2Click(Sender: TObject);
begin
  c1.RandomValue;
  UpdateC1ToGui;
end;

procedure TForm1.UpdateC1FromGui;
begin
  if fDisableSet then exit;
  c1.Lucky := vC1Lucky.Value;
  c1.Dawn := vC1Dawn.Value;
  c1.HitRate := vC1hitrate.Value;
  c1.Escape := vC1escape.Value;
  c1.MagEscape := vC1magescape.Value;
  c1.Ac1 := vC1Ac1.Value;
  c1.Ac2 := vC1Ac2.Value;
  c1.Dc1 := vC1Dc1.Value;
  c1.Dc2 := vC1Dc2.Value;
  c1.Sc1 := vC1Sc1.Value;
  c1.Sc2 := vC1Sc2.Value;
  c1.Mc1 := vC1Mc1.Value;
  c1.Mc2 := vC1Mc2.Value;
  c1.Mac1 := vC1Mac1.Value;
  c1.Mac2 := vC1Mac2.Value;
end;

procedure TForm1.UpdateC1ToGui;
begin
  fDisableSet := true;
  vC1Lucky.Value := c1.Lucky;
  vC1Dawn.Value := c1.Dawn;
  vC1HitRate.Value := c1.hitrate;
  vC1Escape.Value := c1.escape;
  vC1MagEscape.Value := c1.magescape;
  vC1Ac1.Value := c1.Ac1;
  vC1Ac2.Value := c1.Ac2;
  vC1Dc1.Value := c1.Dc1;
  vC1Dc2.Value := c1.Dc2;
  vC1Sc1.Value := c1.Sc1;
  vC1Sc2.Value := c1.Sc2;
  vC1Mc1.Value := c1.Mc1;
  vC1Mc2.Value := c1.Mc2;
  vC1Mac1.Value := c1.Mac1;
  vC1Mac2.Value := c1.Mac2;
  fDisableSet := false;
end;

procedure TForm1.UpdateC2FromGui;
begin
  if fDisableSet then exit;
  c2.Lucky := vC2Lucky.Value;
  c2.Dawn := vC2Dawn.Value;
  c2.HitRate := vC2hitrate.Value;
  c2.Escape := vC2escape.Value;
  c2.MagEscape := vC2magescape.Value;
  c2.Ac1 := vC2Ac1.Value;
  c2.Ac2 := vC2Ac2.Value;
  c2.Dc1 := vC2Dc1.Value;
  c2.Dc2 := vC2Dc2.Value;
  c2.Sc1 := vC2Sc1.Value;
  c2.Sc2 := vC2Sc2.Value;
  c2.Mc1 := vC2Mc1.Value;
  c2.Mc2 := vC2Mc2.Value;
  c2.Mac1 := vC2Mac1.Value;
  c2.Mac2 := vC2Mac2.Value;
end;

procedure TForm1.UpdateC2ToGui;
begin
  fDisableSet := true;
  vC2Lucky.Value := c2.Lucky;
  vC2Dawn.Value := c2.Dawn;
  vC2HitRate.Value := c2.hitrate;
  vC2Escape.Value := c2.escape;
  vC2MagEscape.Value := c2.magescape;
  vC2Ac1.Value := c2.Ac1;
  vC2Ac2.Value := c2.Ac2;
  vC2Dc1.Value := c2.Dc1;
  vC2Dc2.Value := c2.Dc2;
  vC2Sc1.Value := c2.Sc1;
  vC2Sc2.Value := c2.Sc2;
  vC2Mc1.Value := c2.Mc1;
  vC2Mc2.Value := c2.Mc2;
  vC2Mac1.Value := c2.Mac1;
  vC2Mac2.Value := c2.Mac2;
  fDisableSet := false;
end;

procedure TForm1.Button3Click(Sender: TObject);
begin
  C2.RandomValue;
  UpdateC2ToGui;
end;

procedure TForm1.vC2LuckyChange(Sender: TObject);
begin
  UpdateC2FromGui;
end;

procedure TForm1.Button1Click(Sender: TObject);
var
  i:integer;
begin
  for i := 0 to vAttackTime.Value -1 do
  begin
    if c2.HitTest( c1 ) then
      Listbox1.AddItem( 'C1 Attacked c2', nil )
    else
      ListBox1.AddItem( 'c1 missed', nil );
  end;
end;

procedure TForm1.Button4Click(Sender: TObject);
begin
  listbox1.Clear;
end;

end.
