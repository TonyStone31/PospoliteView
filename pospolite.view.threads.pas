unit Pospolite.View.Threads;

{$mode delphi}{$H+}
{$modeswitch advancedrecords}
{$modeswitch nestedprocvars}

interface

uses
  Classes, SysUtils, dateutils, Forms, Pospolite.View.Basics;

type

  TPLAsyncObjectProc = procedure(const AArguments: array of const) of object;
  TPLAsyncNormalProc = procedure(const AArguments: array of const);
  TPLAsyncNestedProc = procedure(const AArguments: array of const) is nested;
  TPLAsyncPureProc = type TPLAsyncObjectProc;

  { TPLAsyncProc }

  TPLAsyncProc = packed record
  strict private
    FObjectProc: TPLAsyncObjectProc;
    FNormalProc: TPLAsyncNormalProc;
    FNestedProc: TPLAsyncNestedProc;
  public
    constructor Create(AObjectProc: TPLAsyncObjectProc); overload;
    constructor Create(ANormalProc: TPLAsyncNormalProc); overload;
    constructor Create(ANestedProc: TPLAsyncNestedProc); overload;

    function AsPureProc: TPLAsyncPureProc;

    class operator := (AObjectProc: TPLAsyncObjectProc) r: TPLAsyncProc;
    class operator := (ANormalProc: TPLAsyncNormalProc) r: TPLAsyncProc;
    class operator := (ANestedProc: TPLAsyncNestedProc) r: TPLAsyncProc;
    class operator := (AProc: TPLAsyncProc) r: TPLAsyncObjectProc;
    class operator := (AProc: TPLAsyncProc) r: TPLAsyncNormalProc;
    class operator := (AProc: TPLAsyncProc) r: TPLAsyncNestedProc;
  end;

  TPLAsyncTask = class;

  { TPLAsyncTaskThread }

  TPLAsyncTaskThread = class(TThread)
  private type
    array_of_const = array of TVarRec;
  private
    FOnNotify: TPLAsyncProc;
    FProc: TPLAsyncProc;
    FArguments: array_of_const;
    FTask: TPLAsyncTask;
  public
    constructor CreateAsyncThread(AProc: TPLAsyncProc);

    procedure Execute; override;

    property OnNotify: TPLAsyncProc read FOnNotify write FOnNotify;
    property Arguments: array_of_const read FArguments write FArguments;
  end;

  { IPLAsyncTask }

  IPLAsyncTask = interface
    ['{C843CA18-5EB0-44E5-892E-5CCFC6AEAE6C}']
    procedure Await(const AArguments: array of const; ANotify: TPLAsyncProc);
    procedure Async(const AArguments: array of const);
    procedure Cancel;
    procedure Suspend;
    procedure Resume;
    function IsRunning: TPLBool;
    function IsCancelled: TPLBool;
    function IsFailed: TPLBool;
    function IsSuspended: TPLBool;
  end;

  { TPLAsyncTask }

  TPLAsyncTask = class(TInterfacedObject, IPLAsyncTask)
  private
    FThread: TPLAsyncTaskThread;
    FRunning: TPLBool;
    FCancelled: TPLBool;
    FFailed: TPLBool;

    procedure Fail;
  public
    constructor Create(AProc: TPLAsyncProc);

    procedure Await(const AArguments: array of const; ANotify: TPLAsyncProc);
    procedure Async(const AArguments: array of const);
    procedure Cancel;
    procedure Suspend;
    procedure Resume;
    function IsRunning: TPLBool; inline;
    function IsCancelled: TPLBool; inline;
    function IsFailed: TPLBool; inline;
    function IsSuspended: TPLBool; inline;
  end;

  // Run async task with parameters and without await
  function Async(AProc: TPLAsyncProc; const AArguments: array of const): IPLAsyncTask; overload;
  // Run async task without parameters and without await
  function Async(AProc: TPLAsyncProc): IPLAsyncTask; overload; inline;
  // Run async task with parameters and await
  function Await(AProc, ANotify: TPLAsyncProc; const AArguments: array of const): IPLAsyncTask; overload;
  // Run async task without parameters and with await
  function Await(AProc, ANotify: TPLAsyncProc): IPLAsyncTask; overload; inline;

implementation

function Async(AProc: TPLAsyncProc; const AArguments: array of const): IPLAsyncTask;
begin
  Result := TPLAsyncTask.Create(AProc);
  Result.Async(AArguments);
end;

function Async(AProc: TPLAsyncProc): IPLAsyncTask;
begin
  Result := Async(AProc, []);
end;

function Await(AProc, ANotify: TPLAsyncProc;
  const AArguments: array of const): IPLAsyncTask;
begin
  Result := TPLAsyncTask.Create(AProc);
  Result.Await(AArguments, ANotify);
end;

function Await(AProc, ANotify: TPLAsyncProc): IPLAsyncTask;
begin
  Result := Await(AProc, ANotify, []);
end;

{ TPLAsyncProc }

constructor TPLAsyncProc.Create(AObjectProc: TPLAsyncObjectProc);
begin
  FObjectProc := AObjectProc;
  FNormalProc := nil;
  FNestedProc := nil;
end;

constructor TPLAsyncProc.Create(ANormalProc: TPLAsyncNormalProc);
begin
  FObjectProc := nil;
  FNormalProc := ANormalProc;
  FNestedProc := nil;
end;

constructor TPLAsyncProc.Create(ANestedProc: TPLAsyncNestedProc);
begin
  FObjectProc := nil;
  FNormalProc := nil;
  FNestedProc := ANestedProc;
end;

function TPLAsyncProc.AsPureProc: TPLAsyncPureProc;
begin
  if Assigned(FObjectProc) then Result := TPLAsyncPureProc(FObjectProc)
  else if Assigned(FNormalProc) then Result := TPLAsyncPureProc((@FNormalProc)^)
  else if Assigned(FNestedProc) then Result := TPLAsyncPureProc(FNestedProc)
  else Result := nil;
end;

class operator TPLAsyncProc.:=(AObjectProc: TPLAsyncObjectProc) r: TPLAsyncProc;
begin
  r := TPLAsyncProc.Create(AObjectProc);
end;

class operator TPLAsyncProc.:=(ANormalProc: TPLAsyncNormalProc) r: TPLAsyncProc;
begin
  r := TPLAsyncProc.Create(ANormalProc);
end;

class operator TPLAsyncProc.:=(ANestedProc: TPLAsyncNestedProc) r: TPLAsyncProc;
begin
  r := TPLAsyncProc.Create(ANestedProc);
end;

class operator TPLAsyncProc.:=(AProc: TPLAsyncProc) r: TPLAsyncObjectProc;
begin
  r := AProc.FObjectProc;
end;

class operator TPLAsyncProc.:=(AProc: TPLAsyncProc) r: TPLAsyncNormalProc;
begin
  r := AProc.FNormalProc;
end;

class operator TPLAsyncProc.:=(AProc: TPLAsyncProc) r: TPLAsyncNestedProc;
begin
  r := AProc.FNestedProc;
end;

{ TPLAsyncTaskThread }

constructor TPLAsyncTaskThread.CreateAsyncThread(AProc: TPLAsyncProc);
begin
  inherited Create(true);

  FProc := AProc;
  FOnNotify := nil;
  FreeOnTerminate := true;
end;

procedure TPLAsyncTaskThread.Execute;
begin
  try
    if Assigned(FProc.AsPureProc) then FProc.AsPureProc()(FArguments);
  except
    on e: exception do FTask.Fail;
  end;

  if Assigned(FOnNotify.AsPureProc) then FOnNotify.AsPureProc()(FArguments);
end;

{ TPLAsyncTask }

procedure TPLAsyncTask.Fail;
begin
  FRunning := false;
  FCancelled := false;
  FFailed := true;
end;

constructor TPLAsyncTask.Create(AProc: TPLAsyncProc);
begin
  inherited Create;

  FThread := TPLAsyncTaskThread.CreateAsyncThread(AProc);
  FThread.FTask := self;
  FRunning := false;
  FCancelled := false;
  FFailed := false;
end;

procedure TPLAsyncTask.Await(const AArguments: array of const;
  ANotify: TPLAsyncProc);
var
  i: SizeInt;
begin
  if FFailed then exit;

  try
    FRunning := true;
    FCancelled := false;

    SetLength(FThread.FArguments, Length(AArguments));
    for i := Low(AArguments) to High(FThread.Arguments) do
      FThread.Arguments[i] := AArguments[i];

    FThread.Start;
  except
    on e: exception do Fail;
  end;
end;

procedure TPLAsyncTask.Async(const AArguments: array of const);
begin
  Self.Await(AArguments, nil);
end;

procedure TPLAsyncTask.Cancel;
begin
  if FRunning then begin
    FThread.Terminate;
    FCancelled := FThread.CheckTerminated;
    if FCancelled then FRunning := false;
  end;
end;

procedure TPLAsyncTask.Suspend;
begin
  if FRunning and not FThread.Suspended then FThread.Suspended := true;
end;

procedure TPLAsyncTask.Resume;
begin
  if FRunning and FThread.Suspended then FThread.Suspended := false;
end;

function TPLAsyncTask.IsRunning: TPLBool;
begin
  Result := FRunning;
end;

function TPLAsyncTask.IsCancelled: TPLBool;
begin
  Result := FCancelled;
end;

function TPLAsyncTask.IsFailed: TPLBool;
begin
  Result := FFailed;
end;

function TPLAsyncTask.IsSuspended: TPLBool;
begin
  Result := FThread.Suspended;
end;

end.

