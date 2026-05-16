unit define;

interface
uses
  Windows, Math;
type
  TWooolObject = class( TObject )
  private
    Flucky:integer;
    Fdawn:integer;
    Fdc1:integer;
    Fdc2:integer;
    Fmc1:integer;
    Fmc2:integer;
    Fsc1:integer;
    Fsc2:integer;
    Fac1:integer;
    Fac2:integer;
    Fmac1:integer;
    Fmac2:integer;
    Fhitrate:integer;
    Fescape:integer;
    Fmagescape:integer;
  public
    property Lucky:integer read Flucky write Flucky;
    property Dawn:integer read Fdawn write Fdawn;
    property Dc1:integer read FDc1 write FDc1;
    property Dc2:integer read FDc2 write FDc2;
    property Mc1:integer read FMc1 write FMc1;
    property Mc2:integer read FMc2 write FMc2;
    property Sc1:integer read FSc1 write FSc1;
    property Sc2:integer read FSc2 write FSc2;
    property Ac1:integer read FAc1 write FAc1;
    property Ac2:integer read FAc2 write FAc2;
    property Mac1:integer read FMac1 write FMac1;
    property Mac2:integer read FMac2 write FMac2;
    property hitrate:integer read Fhitrate write Fhitrate;
    property escape:integer read Fescape write Fescape;
    property magescape:integer read Fmagescape write Fmagescape;
    function GetPropPower( nMin, nMax:integer ):integer;
    function GetMagicAttackDamage( nDamage:integer ):integer;
    function GetPhysicsAttackDamage( nDamage:integer ):integer;
    function HitTest( Attacker:TWooolObject ):Boolean;
    procedure RandomValue;
  end;

  function GetRangeRand( nMin, nMax:integer ):integer;

implementation
function GetRangeRand( nMin, nMax:integer ):integer;
var
  iMax, iMin:integer;
begin
  iMax := Max(nMin, nMax );
  iMin := Min(nMin, nMax );

  Result := Random( iMax - iMin + 1 ) + iMin;
end;
function TWooolObject.GetPropPower( nMin, nMax:integer ):integer;
var
  ilucky:integer;
begin
		//int lucky = GetPropValue( PI_LUCKY ) - GetPropValue( PI_DAWN );
    ilucky := Lucky - Dawn;

		if iLucky < 0 then
    begin
			nMax := nMin + (nMax-nMin)* Max( ilucky + 10, 0 ) div 10;
    end
		else
    begin
      if iLucky > 0 then
	  		nMin := nMax - (nMax-nMin)*Max( 10 - ilucky, 0 ) div 10;
    end;
		Result := GetRangeRand( nMin, nMax );

end;

function TWooolObject.GetMagicAttackDamage( nDamage:integer ):integer;
var
  vMac:integer;
begin
  vMac := GetPropPower( Mac1, Mac2 );
  if nDamage < vMac then
  begin
    Result := Random( 2 );
  end
  else
    Result := nDamage - vMac;
end;

function TWooolObject.GetPhysicsAttackDamage( nDamage:integer ):integer;
var
  vAc:integer;
begin
  vAc := GetPropPower( Ac1, Ac2 );
  if nDamage < vAc then
    Result := Random( 2 )
  else
    Result := nDamage - vAc;
end;

function TWooolObject.HitTest( Attacker:TWooolObject ):Boolean;
var
  vHitRate:integer;
  vEscape:integer;
begin
  Result := false;
  vHitRate := Attacker.HitRate;
  vEscape := Escape;
  if vHitRate < Random( vEscape ) then exit;
  Result := true;
end;

procedure TWooolObject.RandomValue;
begin
  HitRate := Random( 5 );
  Escape := Random( 10 );
  MagEscape := Random( 2 );
  Lucky := Random( 2 );
  Dawn := Random( 2 );
  Ac1 := Random( 5 );
  Ac2 := Ac1 + Random( 5 );
  Mac1 := Random( 7 );
  Mac2 := Mac1 + Random( 7 );
  Dc1 := Random( 10 );
  Dc2 := Dc1 + Random( 10 );
  Mc1 := Random( 9 );
  Mc2 := Mc1 + Random( 9 );
  Sc1 := Random( 6 );
  Sc2 := Sc1 + Random( 6 );
end;

end.
