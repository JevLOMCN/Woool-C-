object Form1: TForm1
  Left = 185
  Top = 148
  Width = 629
  Height = 397
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #26032#23435#20307
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object Label2: TLabel
    Left = 200
    Top = 20
    Width = 48
    Height = 12
    Caption = #25915#20987#27425#25968
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 185
    Height = 353
    Caption = #35282#33394#30002
    TabOrder = 0
    object Gauge1: TGauge
      Left = 56
      Top = 24
      Width = 121
      Height = 17
      BackColor = clGray
      ForeColor = clRed
      Progress = 50
    end
    object Label1: TLabel
      Left = 16
      Top = 61
      Width = 24
      Height = 12
      Caption = #24184#36816
    end
    object v: TLabel
      Left = 16
      Top = 85
      Width = 24
      Height = 12
      Caption = #35781#21650
    end
    object Label3: TLabel
      Left = 16
      Top = 109
      Width = 12
      Height = 12
      Caption = 'AC'
    end
    object Label8: TLabel
      Left = 16
      Top = 229
      Width = 24
      Height = 12
      Caption = #21629#20013
    end
    object Label9: TLabel
      Left = 16
      Top = 253
      Width = 24
      Height = 12
      Caption = #36530#36991
    end
    object Label10: TLabel
      Left = 16
      Top = 277
      Width = 48
      Height = 12
      Caption = #39764#27861#36530#36991
    end
    object Label4: TLabel
      Left = 112
      Top = 108
      Width = 6
      Height = 12
      Caption = '-'
    end
    object Label5: TLabel
      Left = 16
      Top = 133
      Width = 18
      Height = 12
      Caption = 'MAC'
    end
    object Label6: TLabel
      Left = 112
      Top = 132
      Width = 6
      Height = 12
      Caption = '-'
    end
    object Label7: TLabel
      Left = 16
      Top = 157
      Width = 12
      Height = 12
      Caption = 'DC'
    end
    object Label11: TLabel
      Left = 112
      Top = 156
      Width = 6
      Height = 12
      Caption = '-'
    end
    object Label12: TLabel
      Left = 16
      Top = 181
      Width = 12
      Height = 12
      Caption = 'MC'
    end
    object Label13: TLabel
      Left = 112
      Top = 180
      Width = 6
      Height = 12
      Caption = '-'
    end
    object Label14: TLabel
      Left = 16
      Top = 205
      Width = 12
      Height = 12
      Caption = 'SC'
    end
    object Label15: TLabel
      Left = 112
      Top = 204
      Width = 6
      Height = 12
      Caption = '-'
    end
    object vC1Lucky: TSpinEdit
      Left = 56
      Top = 56
      Width = 121
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 0
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Dawn: TSpinEdit
      Left = 56
      Top = 80
      Width = 121
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 1
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Ac1: TSpinEdit
      Left = 56
      Top = 104
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 2
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Hitrate: TSpinEdit
      Left = 56
      Top = 224
      Width = 121
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 3
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Escape: TSpinEdit
      Left = 56
      Top = 248
      Width = 121
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 4
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1MagEscape: TSpinEdit
      Left = 72
      Top = 272
      Width = 105
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 5
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Ac2: TSpinEdit
      Left = 128
      Top = 104
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 6
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Mac1: TSpinEdit
      Left = 56
      Top = 128
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 7
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Mac2: TSpinEdit
      Left = 128
      Top = 128
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 8
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Dc1: TSpinEdit
      Left = 56
      Top = 152
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 9
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Dc2: TSpinEdit
      Left = 128
      Top = 152
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 10
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Mc1: TSpinEdit
      Left = 56
      Top = 176
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 11
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Mc2: TSpinEdit
      Left = 128
      Top = 176
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 12
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Sc1: TSpinEdit
      Left = 56
      Top = 200
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 13
      Value = 0
      OnChange = vC1LuckyChange
    end
    object vC1Sc2: TSpinEdit
      Left = 128
      Top = 200
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 14
      Value = 0
      OnChange = vC1LuckyChange
    end
    object Button2: TButton
      Left = 48
      Top = 304
      Width = 75
      Height = 25
      Caption = 'Button2'
      TabOrder = 15
      OnClick = Button2Click
    end
  end
  object vAttackTime: TSpinEdit
    Left = 256
    Top = 16
    Width = 65
    Height = 21
    MaxValue = 0
    MinValue = 0
    TabOrder = 1
    Value = 0
  end
  object GroupBox2: TGroupBox
    Left = 432
    Top = 8
    Width = 185
    Height = 353
    Caption = #35282#33394#20057
    TabOrder = 2
    object Gauge2: TGauge
      Left = 56
      Top = 24
      Width = 121
      Height = 17
      BackColor = clGray
      ForeColor = clRed
      Progress = 50
    end
    object Label16: TLabel
      Left = 16
      Top = 61
      Width = 24
      Height = 12
      Caption = #24184#36816
    end
    object Label17: TLabel
      Left = 16
      Top = 85
      Width = 24
      Height = 12
      Caption = #35781#21650
    end
    object Label18: TLabel
      Left = 16
      Top = 109
      Width = 12
      Height = 12
      Caption = 'AC'
    end
    object Label19: TLabel
      Left = 16
      Top = 229
      Width = 24
      Height = 12
      Caption = #21629#20013
    end
    object Label20: TLabel
      Left = 16
      Top = 253
      Width = 24
      Height = 12
      Caption = #36530#36991
    end
    object Label21: TLabel
      Left = 16
      Top = 277
      Width = 48
      Height = 12
      Caption = #39764#27861#36530#36991
    end
    object Label22: TLabel
      Left = 112
      Top = 108
      Width = 6
      Height = 12
      Caption = '-'
    end
    object Label23: TLabel
      Left = 16
      Top = 133
      Width = 18
      Height = 12
      Caption = 'MAC'
    end
    object Label24: TLabel
      Left = 112
      Top = 132
      Width = 6
      Height = 12
      Caption = '-'
    end
    object Label25: TLabel
      Left = 16
      Top = 157
      Width = 12
      Height = 12
      Caption = 'DC'
    end
    object Label26: TLabel
      Left = 112
      Top = 156
      Width = 6
      Height = 12
      Caption = '-'
    end
    object Label27: TLabel
      Left = 16
      Top = 181
      Width = 12
      Height = 12
      Caption = 'MC'
    end
    object Label28: TLabel
      Left = 112
      Top = 180
      Width = 6
      Height = 12
      Caption = '-'
    end
    object Label29: TLabel
      Left = 16
      Top = 205
      Width = 12
      Height = 12
      Caption = 'SC'
    end
    object Label30: TLabel
      Left = 112
      Top = 204
      Width = 6
      Height = 12
      Caption = '-'
    end
    object vC2Lucky: TSpinEdit
      Left = 56
      Top = 56
      Width = 121
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 0
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Dawn: TSpinEdit
      Left = 56
      Top = 80
      Width = 121
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 1
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Ac1: TSpinEdit
      Left = 56
      Top = 104
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 2
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Hitrate: TSpinEdit
      Left = 56
      Top = 224
      Width = 121
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 3
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Escape: TSpinEdit
      Left = 56
      Top = 248
      Width = 121
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 4
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2MagEscape: TSpinEdit
      Left = 72
      Top = 272
      Width = 105
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 5
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Ac2: TSpinEdit
      Left = 128
      Top = 104
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 6
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Mac1: TSpinEdit
      Left = 56
      Top = 128
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 7
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Mac2: TSpinEdit
      Left = 128
      Top = 128
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 8
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Dc1: TSpinEdit
      Left = 56
      Top = 152
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 9
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Dc2: TSpinEdit
      Left = 128
      Top = 152
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 10
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Mc1: TSpinEdit
      Left = 56
      Top = 176
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 11
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Mc2: TSpinEdit
      Left = 128
      Top = 176
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 12
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Sc1: TSpinEdit
      Left = 56
      Top = 200
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 13
      Value = 0
      OnChange = vC2LuckyChange
    end
    object vC2Sc2: TSpinEdit
      Left = 128
      Top = 200
      Width = 49
      Height = 21
      MaxLength = 10
      MaxValue = 255
      MinValue = 0
      TabOrder = 14
      Value = 0
      OnChange = vC2LuckyChange
    end
    object Button3: TButton
      Left = 64
      Top = 304
      Width = 75
      Height = 25
      Caption = 'Button3'
      TabOrder = 15
      OnClick = Button3Click
    end
  end
  object ListBox1: TListBox
    Left = 200
    Top = 88
    Width = 225
    Height = 273
    ItemHeight = 12
    TabOrder = 3
  end
  object Button1: TButton
    Left = 344
    Top = 16
    Width = 75
    Height = 25
    Caption = 'Button1'
    TabOrder = 4
    OnClick = Button1Click
  end
  object Button4: TButton
    Left = 352
    Top = 64
    Width = 75
    Height = 25
    Caption = 'Button4'
    TabOrder = 5
    OnClick = Button4Click
  end
end
