unit clMain;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, SUIForm, StdCtrls, SUIMemo, SUIImagePanel, SUIButton, define,
  CheckLst, SUICheckListBox, ComCtrls, SUIStatusBar;

type
  TfrmMain = class(TForm)
    suiForm1: TsuiForm;
    suiMemo1: TsuiMemo;
    imgBG: TImage;
    suiStatusBar1: TsuiStatusBar;
    imgPage: TImage;
    procedure suiButton1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure suiMemo1Change(Sender: TObject);
  private
    drawpage:TDrawPage;
    { Private declarations }
  public
    { Public declarations }
  end;

var
  frmMain: TfrmMain;

implementation

{$R *.dfm}

procedure TfrmMain.suiButton1Click(Sender: TObject);
begin
  //Image1.Repaint;
  imgPage.Canvas.CopyRect(imgBG.Canvas.ClipRect, imgBG.Canvas, imgBG.Canvas.ClipRect);
  drawpage.ParsePageToCanvas( imgPage.Canvas, 32, 18, suiMemo1.Text );

end;

procedure TfrmMain.FormCreate(Sender: TObject);
begin
  imgPage.Left := imgBG.Left;
  imgPage.Top := imgBG.Top;
  imgPage.Width := imgBG.Width;
  imgPage.Height := imgBG.Height;
  drawpage := TDrawPage.Create;
  imgPage.Canvas.Font := frmMain.Font;
  imgPage.Canvas.CopyRect(imgBG.Canvas.ClipRect, imgBG.Canvas, imgBG.Canvas.ClipRect);

end;

procedure TfrmMain.suiMemo1Change(Sender: TObject);
begin
  imgPage.Canvas.CopyRect(imgBG.Canvas.ClipRect, imgBG.Canvas, imgBG.Canvas.ClipRect);
  drawpage.ParsePageToCanvas( imgPage.Canvas, 32, 18, suiMemo1.Text );
  suiStatusBar1.Panels.Items[0].Text := 'ÁÐ: ' + IntToStr(suiMemo1.CaretPos.x ) + ' ÐÐ: ' +
  IntToStr( suiMemo1.CaretPos.y );
end;

end.
