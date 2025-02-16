{ TPLNumberRange }

constructor TPLNumberRange.Create(const AMin, AMax: T);
begin
  min := AMin;
  max := AMax;
end;

class function TPLNumberRange.InRanges(const AValue: T;
  const ARanges: array of TPLNumberRangeT): TPLBool;
var
  i: SizeInt;
begin
  Result := false;

  for i := Low(ARanges) to High(ARanges) do begin
    if (ARanges[i].min <= AValue) and (AValue <= ARanges[i].max) then exit(true);
  end;
end;

function TPLNumberRange.Correct(const ANumber: Variant): T;
begin
  if ANumber < min then Result := min else
  if ANumber > max then Result := max else
  Result := T(ANumber);
end;

{ TPLListEnumerator }

function TPLListEnumerator.GetCurrent: E;
begin
  Result := FList[FIndex];
end;

constructor TPLListEnumerator.Create(const AList: L);
begin
  inherited Create;

  FList := AList;
  FIndex := -1;
end;

function TPLListEnumerator.MoveNext: TPLBool;
begin
  if FIndex+1 < FList.Count then begin
    Result := true;
    Inc(FIndex);
  end else Result := false;
end;

procedure TPLListEnumerator.Reset;
begin
  FIndex := 0;
end;

{ TPLObjectList }

function TPLObjectList.GetItem(AIndex: SizeInt): T;
begin
  if (AIndex < 0) or (AIndex >= FSize) then
    Result := nil
  else
    Result := FArray[AIndex];
end;

procedure TPLObjectList.SetItem(AIndex: SizeInt; AValue: T);
begin
  if (AIndex < 0) or (AIndex >= FSize) then exit;

  FArray[AIndex] := AValue;
end;

function TPLObjectList.DefaultCompare(a, b: T): TPLBool;
begin
  Result := a = b;
end;

procedure TPLObjectList.MergeArray(var AArray: TListOfT; l, r, x, y: TPLInt;
  AComparator: specialize TPLObjectListSortCompare<T>);
var
  i, j, k, s: TPLInt;
  c: TListOfT;
  a: TListOfT absolute AArray;
begin
  i := l;
  j := y;
  k := 0;
  SetLength(c, r - l + 1 + y - x + 1);

  while (l <= r) and (x <= y) do begin
    if AComparator(a[l], a[x]) = -1 then begin
      c[k] := a[l];
      Inc(l);
    end else begin
      c[k] := a[x];
      Inc(x);
    end;
    Inc(k);
  end;

  if l <= r then for s := l to r do begin
    c[k] := a[s];
    Inc(k);
  end else for s := x to y do begin
    c[k] := a[s];
    Inc(k);
  end;

  k := 0;
  for s := i to j do begin
    a[s] := c[k];
    Inc(k);
  end;
end;

procedure TPLObjectList.SortArray(AArray: TListOfT; ALeft, ARight: TPLInt;
  AComparator: specialize TPLObjectListSortCompare<T>);
var
  m: TPLInt;
begin
  if ALeft >= ARight then exit;

  m := (ALeft + ARight) div 2;
  SortArray(AArray, ALeft, m, AComparator);
  SortArray(AArray, m + 1, ARight, AComparator);
  MergeArray(AArray, ALeft, m, m + 1, ARight, AComparator);
end;

function TPLObjectList.GetEnumerator: IObjectListEnumerator;
begin
  Result := TObjectListEnumerator.Create(Self);
end;

function TPLObjectList.Find(AItem: T; AComparator: specialize
  TPLObjectListFindCompare<T>): SizeInt;
var
  i: SizeInt;
begin
  Result := -1;
  if not Assigned(AComparator) then AComparator := @DefaultCompare;

  for i := 0 to FSize-1 do
    if AComparator(FArray[i], AItem) then exit(i);
end;

procedure TPLObjectList.Sort(AComparator: specialize TPLObjectListSortCompare<T>);
begin
  if not Assigned(AComparator) then exit;

  SortArray(FArray, 0, Length(FArray)-1, AComparator);
end;

constructor TPLObjectList.Create(AFreeObjects: TPLBool);
begin
  inherited Create;

  FSize := 0;
  FFreeObjects := AFreeObjects;
  SetLength(FArray, 0);
end;

destructor TPLObjectList.Destroy;
begin
  Clear;

  inherited Destroy;
end;

procedure TPLObjectList.Add(AItem: T);
begin
  if FSize >= MaxListSize then exit;

  SetLength(FArray, FSize + 1);
  FArray[FSize] := AItem;
  FSize += 1;
end;

procedure TPLObjectList.CustomInsert(AIndex: SizeInt; AItem, ATempItem: T);
var
  i: SizeInt;
begin
  if Empty then begin
    Add(AItem);
    if Assigned(ATempItem) then ATempItem.Free;
    exit;
  end;

  if AIndex < 0 then AIndex := 0
  else if AIndex > Count-1 then AIndex := max(0, Count);

  if AIndex < Count then begin
    SetLength(FArray, Count+1);
    FArray[Count] := ATempItem;

    for i := High(FArray) downto AIndex do
      FArray[i] := FArray[i-1];

    FArray[AIndex] := AItem;
    FSize += 1;
  end else Add(AItem);

  if Assigned(ATempItem) then ATempItem.Free;
end;

procedure TPLObjectList.Insert(AIndex: SizeInt; AItem: T);
begin
  CustomInsert(AIndex, AItem, nil);
end;

procedure TPLObjectList.Replace(AIndex1, AIndex2: SizeInt);
var
  pom: T;
begin
  if (AIndex1 = AIndex2) or (AIndex1 < 0) or (AIndex2 < 0) or (AIndex1 >= Count)
    or (AIndex2 >= Count) then exit;

  pom := Item[AIndex1];
  Item[AIndex1] := Item[AIndex2];
  Item[AIndex2] := pom;
end;

procedure TPLObjectList.Remove(AItem: T);
var
  id, i: SizeInt;
begin
  id := Find(AItem);

  if id < 0 then exit;
  if FFreeObjects then FArray[id].Free;

  for i := id to FSize-2 do
    FArray[i] := FArray[i + 1];

  FSize := FSize - 1;
  SetLength(FArray, FSize);
end;

function TPLObjectList.Count: SizeInt;
begin
  Result := FSize;
end;

function TPLObjectList.Empty: TPLBool;
begin
  Result := FSize = 0;
end;

procedure TPLObjectList.Clear;
var
  i: integer;
begin
  if FFreeObjects then begin
    for i := 0 to FSize-1 do FArray[i].Free;
  end;

  FSize := 0;
  SetLength(FArray, FSize);
end;

function TPLObjectList.Last: T;
begin
  Result := GetItem(FSize - 1);
end;

function TPLObjectList.First: T;
begin
  Result := GetItem(0);
end;

function TPLObjectList.Poll: T;
begin
  Result := First;

  if not Empty then Remove(First);
end;

function TPLObjectList.Pop: T;
begin
  Result := Last;

  if not Empty then Remove(Last);
end;

function TPLObjectList.Duplicate: specialize IPLObjectList<T>;
var
  x: T;
begin
  Result := TPLObjectList.Create(FFreeObjects);

  for x in self do Result.Add(x);
end;

procedure TPLObjectList.SetObjectsFreeing(const AValue: TPLBool);
begin
  FFreeObjects := AValue;
end;

{ TPLList }

function TPLList.GetItem(AIndex: SizeInt): T;
begin
  if (AIndex < 0) or (AIndex >= FSize) then
    Result := Default(T)
  else
    Result := FArray[AIndex];
end;

function TPLList.GetData: Pointer;
begin
  if FSize > 0 then Result := @FArray[0] else Result := nil;
end;

procedure TPLList.SetItem(AIndex: SizeInt; AValue: T);
begin
  if (AIndex < 0) or (AIndex >= FSize) then exit;

  FArray[AIndex] := AValue;
end;

function TPLList.GetEnumerator: IListEnumerator;
begin
  Result := TListEnumerator.Create(Self);
end;

constructor TPLList.Create;
begin
  inherited Create;

  FSize := 0;
  SetLength(FArray, 0);
end;

destructor TPLList.Destroy;
begin
  Clear;

  inherited Destroy;
end;

procedure TPLList.Add(AItem: T);
begin
  if FSize >= MaxListSize then exit;

  SetLength(FArray, FSize + 1);
  FArray[FSize] := AItem;
  FSize += 1;
end;

procedure TPLList.Remove(AItem: T);
var
  id, i: SizeInt;
begin
  id := Find(AItem);

  if id < 0 then exit;

  for i := id to FSize-2 do
    FArray[i] := FArray[i + 1];

  FSize := FSize - 1;
  SetLength(FArray, FSize);
end;

function TPLList.Find(AItem: T): SizeInt;
var
  i: SizeInt;
begin
  Result := -1;
  for i := 0 to FSize-1 do
    if FArray[i] = AItem then exit(i);
end;

function TPLList.Count: SizeInt;
begin
  Result := FSize;
end;

function TPLList.Empty: TPLBool;
begin
  Result := FSize = 0;
end;

procedure TPLList.Clear;
begin
  FSize := 0;
  SetLength(FArray, FSize);
end;

function TPLList.Last: T;
begin
  Result := GetItem(FSize - 1);
end;

function TPLList.First: T;
begin
  Result := GetItem(0);
end;

function TPLList.Poll: T;
begin
  Result := First;

  if not Empty then Remove(First);
end;

function TPLList.Pop: T;
begin
  Result := Last;

  if not Empty then Remove(Last);
end;

function TPLList.Duplicate: specialize IPLList<T>;
var
  x: T;
begin
  Result := TPLList.Create;

  for x in self do Result.Add(x);
end;

{ TPLInterfaceList }

procedure TPLInterfaceList.Add(AItem: I);
begin
  AItem._AddRef;
  inherited Add(AItem);
end;

procedure TPLInterfaceList.Remove(AItem: I);
begin
  AItem._Release;
  inherited Remove(AItem);
end;

procedure TPLInterfaceList.Clear;
var
  x: SizeInt;
begin
  for x := Low(FArray) to High(FArray) do
    FArray[x]._Release;

  inherited Clear;
end;

function TPLInterfaceList.Duplicate: specialize IPLInterfaceList<I>;
var
  x: I;
begin
  Result := TPLInterfaceList.Create;

  for x in self do Result.Add(x);
end;

{ TPLFuncs }

class procedure TPLFuncs.Swap(var A, B: T);
var
  tmp: T;
begin
  tmp := A;
  A := B;
  B := tmp;
end;

class function TPLFuncs.NewArray(ATab: array of T): specialize TArray<T>;
var
  i: TPLInt;
begin
  SetLength(Result, Length(ATab));
  for i := Low(ATab) to High(ATab) do
    Result[i] := ATab[i];
end;

class function TPLFuncs.NewList(ATab: array of T): specialize TPLList<T>;
var
  i: SizeInt;
begin
  Result := ListHelper.Create;

  for i := Low(ATab) to High(ATab) do
    Result.Add(ATab[i]);
end;

class function TPLFuncs.ToString(ATab: specialize TArray<T>): TPLString;
var
  v: Variant;
  i: SizeInt;
begin
  Result := '[';

  for i := Low(ATab) to High(ATab) do begin
    v := ATab[i];

    if VarIsStr(v) then
      Result += '''' + v + ''''
    else
      Result += VarToStr(v);

    if i < High(ATab) then Result += ', ';
  end;

  Result += ']';
end;

class procedure TPLFuncs.FillArray(var ATab: array of T; const AValue: T);
var
  i: SizeInt;
begin
  for i := Low(ATab) to High(ATab) do
    ATab[i] := AValue;
end;

class function TPLFuncs.Extract(const AList: specialize TPLList<T>): specialize TArray<T>;
begin
  Result := TPLFuncs.NewArray(AList.FArray);
end;

{ TPLFuncsOfClass }

class function TPLFuncsOfClass.DefaultComparator(const AObject: T;
  const ACriteria: Variant): TPLSign;
begin
  case VarCompareValue(ObjectToVariant(AObject), ACriteria) of
    vrEqual: Result := 0;
    vrLessThan: Result := -1;
    else Result := 1;
  end;
end;

class function TPLFuncsOfClass.InternalBinarySearch(AList: ListHelper;
  const ACriteria: Variant; c: ComparatorHelper; ALeft, ARight: SizeInt): SizeInt;
var
  m: SizeInt;
begin
  if ARight >= ALeft then begin
    m := ALeft + (ARight - ALeft) div 2;

    if c(AList[m], ACriteria) = 0 then exit(m);
    if c(AList[m], ACriteria) = -1 then exit(InternalBinarySearch(AList, ACriteria, c, ALeft, m - 1));

    exit(InternalBinarySearch(AList, ACriteria, c, m + 1, ARight));
  end;

  Result := -1;
end;

class function TPLFuncsOfClass.InternalSearch(AList: ListHelper; const ACriteria: Variant;
  c: ComparatorHelper): SizeInt;
var
  bound: SizeInt = 1;
begin
  Result := -1;

  if AList.Count = 0 then exit;

  while (bound < AList.Count) and (c(AList[bound], ACriteria) = -1) do bound *= 2;

  Result := InternalBinarySearch(AList, ACriteria, c, bound div 2, min(bound + 1, AList.Count - 1));
end;

class procedure TPLFuncsOfClass.Swap(var A, B: T);
var
  tmp: T;
begin
  tmp := A;
  A := B;
  B := A;
end;

class function TPLFuncsOfClass.NewList(ATab: array of T; const AFreeObjects: TPLBool): specialize TPLObjectList<T>;
var
  i: SizeInt;
begin
  Result := ListHelper.Create(AFreeObjects);

  for i := Low(ATab) to High(ATab) do
    Result.Add(ATab[i]);
end;

class function TPLFuncsOfClass.NewArray(ATab: array of T): specialize TArray<T>;
var
  i: SizeInt;
begin
  SetLength(Result, Length(ATab));

  for i := Low(ATab) to High(ATab) do
    Result[i] := ATab[i];
end;

class function TPLFuncsOfClass.Extract(const AList: specialize TPLObjectList<T>): specialize TArray<T>;
begin
  Result := TPLFuncsOfClass.NewArray(AList.FArray);
end;

class function TPLFuncsOfClass.FastSearch(AList: specialize TPLObjectList<T>;
  const ACriteria: Variant; ACustomComparator: specialize TPLFuncsOfClassComparator<T>): SizeInt;
begin
  if not Assigned(ACustomComparator) then ACustomComparator := @DefaultComparator;

  Result := InternalSearch(AList, ACriteria, ACustomComparator);
end;

{ TPLParameter }

class operator TPLParameter.=(a, b: TPLParameter) r: TPLBool;
begin
  r := (a.Key = b.Key) and (a.Value = b.Value);
end;

constructor TPLParameter.Create(AKey: K; AValue: V);
begin
  Key := AKey;
  Value := AValue;
end;

{ TPLHTMLObjectAttributes }

function TPLHTMLObjectAttributes.GetCharset: TPLHTMLObjectAttribute;
begin
  Result := FPtrs[7];
end;

function TPLHTMLObjectAttributes.GetClass: TPLHTMLObjectAttribute;
begin
  Result := FPtrs[0];
end;

function TPLHTMLObjectAttributes.GetHref: TPLHTMLObjectAttribute;
begin
  Result := FPtrs[4];
end;

function TPLHTMLObjectAttributes.GetId: TPLHTMLObjectAttribute;
begin
  Result := FPtrs[2];
end;

function TPLHTMLObjectAttributes.GetName: TPLHTMLObjectAttribute;
begin
  Result := FPtrs[1];
end;

function TPLHTMLObjectAttributes.GetRel: TPLHTMLObjectAttribute;
begin
  Result := FPtrs[8];
end;

function TPLHTMLObjectAttributes.GetSrc: TPLHTMLObjectAttribute;
begin
  Result := FPtrs[5];
end;

function TPLHTMLObjectAttributes.GetStyle: TPLHTMLObjectAttribute;
begin
  Result := FPtrs[3];
end;

function TPLHTMLObjectAttributes.GetType: TPLHTMLObjectAttribute;
begin
  Result := FPtrs[6];
end;

procedure TPLHTMLObjectAttributes.UpdateConsts;
begin
  FPtrs[0] := Get('class');
  FPtrs[1] := Get('name');
  FPtrs[2] := Get('id');
  FPtrs[3] := Get('style');
  FPtrs[4] := Get('href');
  FPtrs[5] := Get('src');
  FPtrs[6] := Get('type');
  FPtrs[7] := Get('charset');
  FPtrs[8] := Get('rel');
end;

procedure TPLHTMLObjectAttributes.Add(AItem: TPLHTMLObjectAttribute);
begin
  inherited Add(AItem);
  UpdateConsts;
end;

procedure TPLHTMLObjectAttributes.Remove(AItem: TPLHTMLObjectAttribute);
begin
  inherited Remove(AItem);
  UpdateConsts;
end;

procedure TPLHTMLObjectAttributes.Clear;
begin
  inherited Clear;
  UpdateConsts;
end;

function TPLHTMLObjectAttributes.Get(AName: TPLString): TPLHTMLObjectAttribute;
var
  a: TPLHTMLObjectAttribute;
begin
  AName := AName.Trim.ToLower;

  if AName.IsEmpty then exit(Default(TPLHTMLObjectAttribute));

  for a in Self do
    if a.Key.ToLower = AName then exit(a);

  Result := Default(TPLHTMLObjectAttribute);
end;

function TPLHTMLObjectAttributes.Has(AName: TPLString): TPLBool;
begin
  Result := Get(AName) <> Default(TPLHTMLObjectAttribute);
end;

function TPLHTMLObjectAttributes.ToString: TPLString;
var
  attr: TPLHTMLObjectAttribute;
begin
  Result := '';

  for attr in self do
    Result += attr.Key + '="' + attr.Value + '" ';

  Result := Result.Trim;
end;

{ TPLHTMLObject }

function TPLHTMLObject.GetAttributes: TPLHTMLObjectAttributes;
begin
  Result := FAttributes;
end;

function TPLHTMLObject.GetChild(const AName: TPLString): TPLHTMLObject;
var
  c: TPLHTMLObject;
begin
  for c in FChildren do begin
    if c.Name.ToLower = AName.ToLower then exit(c);
  end;

  Result := nil;
end;

function TPLHTMLObject.GetChildren: TPLHTMLObjects;
begin
  Result := FChildren;
end;

function TPLHTMLObject.GetJSObject: IPLJSBasicObject;
begin
  Result := FJSObject;
end;

function TPLHTMLObject.GetName: TPLString;
begin
  Result := FName;
end;

function TPLHTMLObject.GetNodeType: TPLHTMLObjectNodeType;
begin
  Result := FNodeType;
end;

function TPLHTMLObject.GetParent: TPLHTMLObject;
begin
  Result := FParent;
end;

function TPLHTMLObject.GetPosition: SizeInt;
begin
  Result := FPosition;
end;

function TPLHTMLObject.GetState: TPLCSSElementState;
begin
  Result := FState;
end;

function TPLHTMLObject.GetText: TPLString;
begin
  Result := FText;
end;

function TPLHTMLObject.GetZoom: TPLFloat;
begin
  Result := FZoom;
end;

procedure TPLHTMLObject.SetName(AValue: TPLString);
begin
  AValue := AValue.Trim;
  if (FName = AValue) or (AValue.IsEmpty) then exit;
  FName := AValue;
end;

procedure TPLHTMLObject.SetNodeType(AValue: TPLHTMLObjectNodeType);
begin
  FNodeType := AValue;
end;

procedure TPLHTMLObject.SetParent(AValue: TPLHTMLObject);
begin
  FParent := AValue;
end;

procedure TPLHTMLObject.SetPosition(AValue: SizeInt);
begin
  FPosition := AValue;
end;

procedure TPLHTMLObject.SetState(AValue: TPLCSSElementState);
begin
  FState := AValue;
end;

procedure TPLHTMLObject.SetText(AValue: TPLString);
begin
  FText := AValue;
end;

procedure TPLHTMLObject.SetZoom(AValue: TPLFloat);
begin
  if (AValue = FZoom) or (AValue <= 0) or (AValue > 10) then exit;

  FZoom := AValue;
end;

procedure TPLHTMLObject.DoDraw(ADrawer: Pointer);
begin
  // ...
end;

function TPLHTMLObject.DoToHTMLChildren: TPLString;
var
  obj: TPLHTMLObject;
begin
  Result := '';

  for obj in FChildren do
    Result += obj.ToHTML + LineEnding;
end;

procedure TPLHTMLObject.InitStates;
begin
  // ...
end;

procedure TPLHTMLObject.DoneStates;
begin
  // ...
end;

constructor TPLHTMLObject.Create(AParent: TPLHTMLObject);
begin
  inherited Create;

  FName := 'basic_object';
  FText := '';
  FState := esNormal;
  FZoom := 1;
  FPosition := 0;
  FZIndex := 0;

  FParent := AParent;
  FAttributes := TPLHTMLObjectAttributes.Create;
  FChildren := TPLHTMLObjects.Create;

  InitStates;
end;

destructor TPLHTMLObject.Destroy;
begin
  DoneStates;
  FAttributes.Free;
  FChildren.Free;

  inherited Destroy;
end;

function TPLHTMLObject.Clone: IPLHTMLObject;
begin
  Result := TPLHTMLObject.Create(FParent as TPLHTMLObject);
end;

function TPLHTMLObject.GetCSSProperty(const AName: TPLString;
  AState: TPLCSSElementState; AUseCommonPrefixes: TPLBool): Pointer;
begin
  Result := nil;
end;

function TPLHTMLObject.GetCSSPropertyValue(const AName: TPLString;
  AState: TPLCSSElementState; AUseCommonPrefixes: TPLBool; AIndex: SizeInt
  ): Pointer;
begin
  Result := nil;
end;

procedure TPLHTMLObject.SetCSSPropertyValue(const AName: TPLString;
  const AValue: Pointer; AState: TPLCSSElementState; AIndex: SizeInt);
begin
  // ...
end;

procedure TPLHTMLObject.UpdateScrollbars;
begin
  // ...
end;

procedure TPLHTMLObject.Draw(ADrawer: Pointer);
var
  obj: TPLHTMLObject;
  wr: TPLBool;
begin
  if not isVisible or (Display = 'none') then exit;

  wr := Assigned(Parent) and (Parent.ZIndex <= ZIndex);

  if wr then DoDraw(ADrawer);
  for obj in FChildren do
    obj.Draw(ADrawer);
  if not wr then DoDraw(ADrawer);
end;

function TPLHTMLObject.ToHTML: TPLString;
begin
  Result := DoToHTMLChildren;
end;

function TPLHTMLObject.ToObject: TPLHTMLObject;
begin
  Result := self;
end;

function TPLHTMLObject.PositionInParent: SizeInt;
var
  i: SizeInt;
begin
  Result := -1;
  if not Assigned(FParent) then exit;

  for i := 0 to FParent.Children.Count-1 do begin
    if FParent.Children[i] = self then exit(i);
  end;
end;

procedure TPLHTMLObject.RefreshStyles(const AParentStyles);
begin
  // ...
end;

procedure TPLHTMLObject.UpdateLayout;
var
  obj: TPLHTMLObject;
begin
  if not IsVisible or (Display = 'none') then exit;

  for obj in Children do
    obj.UpdateLayout;

  // size
  for obj in Children do
    obj.UpdateSizeLayout;

  UpdateSizeLayout;

  // position
  UpdatePositionLayout;

  for obj in Children do
    obj.UpdatePositionLayout;
end;

procedure TPLHTMLObject.UpdatePositionLayout;
begin
  // ...
end;

procedure TPLHTMLObject.UpdateSizeLayout;
begin
  // ...
end;

procedure TPLHTMLObject.ApplyInlineStyles;
begin
  // ...
end;

function TPLHTMLObject.GetWidth: TPLFloat;
begin
  Result := 0;
end;

function TPLHTMLObject.GetHeight: TPLFloat;
begin
  Result := 0;
end;

function TPLHTMLObject.GetTop: TPLFloat;
begin
  Result := 0;
end;

function TPLHTMLObject.GetLeft: TPLFloat;
begin
  Result := 0;
end;

function TPLHTMLObject.GetElementTarget: Pointer;
begin
  Result := nil;
end;

function TPLHTMLObject.GetIDFromParent: SizeInt;
begin
  if not Assigned(FParent) then
    Result := -1
  else
    Result := FParent.FChildren.Find(self);
end;

function TPLHTMLObject.IsVisible: TPLBool;
begin
  Result := false;
  // ...
end;

function TPLHTMLObject.Display: TPLString;
begin
  Result := 'block';
  // ...
end;

function TPLHTMLObject.IsLink: TPLBool;
begin
  Result := false;
end;

function TPLHTMLObject.PositionType: TPLString;
begin
  Result := 'static';
end;

function TPLHTMLObject.RealCoords: TPLFloatArray;
begin
  SetLength(Result, 4);

  Result[0] := GetLeft;
  Result[1] := GetTop;
  Result[2] := GetWidth;
  Result[3] := GetHeight;
end;

function TPLHTMLObject.CoordsInObject(const AX, AY: TPLFloat): TPLBool;
var
  c: TPLFloatArray;
begin
  c := RealCoords;

  Result := IsVisible and (Display <> 'none') and (AX >= c[0]) and
    (AX <= c[0] + c[2]) and (AY >= c[1]) and (AY <= c[1] + c[3]);
end;

function TPLHTMLObject.CoordsInObjectOnly(const AX, AY: TPLFloat): TPLBool;
var
  obj: TPLHTMLObject;
begin
  Result := CoordsInObject(AX, AY);

  if Result then begin
    for obj in Children do
      if (obj.ZIndex >= FZIndex) and obj.CoordsInObjectOnly(AX, AY) then exit(false);
  end;
end;

function TPLHTMLObject.CalculateRelativeLength(AValue: TPLFloat; const AUnit: TPLString;
  APropName: TPLString = ''): TPLFloat;
begin
  Result := 0;
end;

procedure TPLHTMLObject.SetEnvironment(AEnvironment: Pointer);
begin
  //
end;

