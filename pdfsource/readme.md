### PDF插件

这是chromium的PDF插件代码，主要是了解他的渲染是如何实现的。这里主要阅读PDF渲染绘制的代码，了解他是如何渲染各种非标准的PDF的。



#### PdfViewWebPlugin

```
这是整个插件的对外实现，上层通过调用这个类实现pdf的加载和渲染。
```

##### 1、相关的继承

###### PDFEngine::Client

```
PDFEngine::Client PDF引擎抽象类中的client接口。 引擎通过回调client类的接口完成各种操作， 如ScrollToX、ScrollToY等操作。
--> PDFiumEngine通过构造函数来维护一个const raw_ptr<PDFEngine::Client> client_， 以此来获取到客户端的行为实现。
raw_ptr指针跟原始指针的行为是一样的，都需要手动去释放。*

相当于PDFEngine需要依赖Client的实现才能完成相关的操作。

比如PDFiumEngine::ScrollToGlobalPoint函数， 获取到当前的滚动条偏移量之后，就会回调client_的ScrollBy（ PdfViewWebPlugin::ScrollBy）方法， 在 PdfViewWebPlugin::ScrollBy方法中又回调了PdfViewWebPlugin::Client中的PostMessage方法来通知上层。


```

###### blink::WebPlugin

```
WebPlugin 浏览器的插件抽象类
```

###### pdf::mojom::PdfListener

```
进程通信框架
```

###### UrlLoader::Client

```
这个应该是请求打开一个网络PDF
```

###### PostMessageReceiver::Client

```
通信插件接口，好像是和外部js的通信
```

###### PaintManager::Client

```
绘制管理器接口
```

###### PdfAccessibilityActionHandler 、PdfAccessibilityImageFetcher

```
这两个好像是辅助显示的相关接口
```



##### 2、相关成员变量和成员函数

###### 

```
kMaximumSavedFileSize 
根据注释可知：保存文档的最大大小

SaveRequestType 加载状态

Client 对外的抽象接口类

```

###### blink::WebPlugin 相关接口

```
  // blink::WebPlugin:
  bool Initialize(blink::WebPluginContainer* container) override;
  void Destroy() override;
  blink::WebPluginContainer* Container() const override;
  v8::Local<v8::Object> V8ScriptableObject(v8::Isolate* isolate) override;
  bool SupportsKeyboardFocus() const override;
  void UpdateAllLifecyclePhases(blink::DocumentUpdateReason reason) override;
  void Paint(cc::PaintCanvas* canvas, const gfx::Rect& rect) override;
  void UpdateGeometry(const gfx::Rect& window_rect,
                      const gfx::Rect& clip_rect,
                      const gfx::Rect& unobscured_rect,
                      bool is_visible) override;
  void UpdateFocus(bool focused, blink::mojom::FocusType focus_type) override;
  void UpdateVisibility(bool visibility) override;
  blink::WebInputEventResult HandleInputEvent(
      const blink::WebCoalescedInputEvent& event,
      ui::Cursor* cursor) override;
  void DidReceiveResponse(const blink::WebURLResponse& response) override;
  void DidReceiveData(const char* data, size_t data_length) override;
  void DidFinishLoading() override;
  void DidFailLoading(const blink::WebURLError& error) override;
  bool SupportsPaginatedPrint() override;
  bool GetPrintPresetOptionsFromDocument(
      blink::WebPrintPresetOptions* print_preset_options) override;
  int PrintBegin(const blink::WebPrintParams& print_params) override;
  void PrintPage(int page_number, cc::PaintCanvas* canvas) override;
  void PrintEnd() override;
  bool HasSelection() const override;
  blink::WebString SelectionAsText() const override;
  blink::WebString SelectionAsMarkup() const override;
  bool CanEditText() const override;
  bool HasEditableText() const override;
  bool CanUndo() const override;
  bool CanRedo() const override;
  bool CanCopy() const override;
  bool ExecuteEditCommand(const blink::WebString& name,
                          const blink::WebString& value) override;
  blink::WebURL LinkAtPosition(const gfx::Point& /*position*/) const override;
  bool StartFind(const blink::WebString& search_text,
                 bool case_sensitive,
                 int identifier) override;
  void SelectFindResult(bool forward, int identifier) override;
  void StopFind() override;
  bool CanRotateView() override;
  void RotateView(blink::WebPlugin::RotationType type) override;

  bool ShouldDispatchImeEventsToPlugin() override;
  blink::WebTextInputType GetPluginTextInputType() override;
  gfx::Rect GetPluginCaretBounds() override;
  void ImeSetCompositionForPlugin(
      const blink::WebString& text,
      const std::vector<ui::ImeTextSpan>& ime_text_spans,
      const gfx::Range& replacement_range,
      int selection_start,
      int selection_end) override;
  void ImeCommitTextForPlugin(
      const blink::WebString& text,
      const std::vector<ui::ImeTextSpan>& ime_text_spans,
      const gfx::Range& replacement_range,
      int relative_cursor_pos) override;
  void ImeFinishComposingTextForPlugin(bool keep_selection) override;
```

###### PDFEngine::Client 相关接口

```
  void ProposeDocumentLayout(const DocumentLayout& layout) override;
  void Invalidate(const gfx::Rect& rect) override;
  void DidScroll(const gfx::Vector2d& offset) override;
  void ScrollToX(int x_screen_coords) override;
  void ScrollToY(int y_screen_coords) override;
  void ScrollBy(const gfx::Vector2d& delta) override;
  void ScrollToPage(int page) override;
  void NavigateTo(const std::string& url,
                  WindowOpenDisposition disposition) override;
  void NavigateToDestination(int page,
                             const float* x,
                             const float* y,
                             const float* zoom) override;
  void UpdateCursor(ui::mojom::CursorType new_cursor_type) override;
  void UpdateTickMarks(const std::vector<gfx::Rect>& tickmarks) override;
  void NotifyNumberOfFindResultsChanged(int total, bool final_result) override;
  void NotifySelectedFindResultChanged(int current_find_index,
                                       bool final_result) override;
  void NotifyTouchSelectionOccurred() override;
  void GetDocumentPassword(
      base::OnceCallback<void(const std::string&)> callback) override;
  void Beep() override;
  void Alert(const std::string& message) override;
  bool Confirm(const std::string& message) override;
  std::string Prompt(const std::string& question,
                     const std::string& default_answer) override;
  std::string GetURL() override;
  void Email(const std::string& to,
             const std::string& cc,
             const std::string& bcc,
             const std::string& subject,
             const std::string& body) override;
  void Print() override;
  void SubmitForm(const std::string& url,
                  const void* data,
                  int length) override;
  std::unique_ptr<UrlLoader> CreateUrlLoader() override;
  v8::Isolate* GetIsolate() override;
  std::vector<SearchStringResult> SearchString(const char16_t* string,
                                               const char16_t* term,
                                               bool case_sensitive) override;
  void DocumentLoadComplete() override;
  void DocumentLoadFailed() override;
  void DocumentHasUnsupportedFeature(const std::string& feature) override;
  void DocumentLoadProgress(uint32_t available, uint32_t doc_size) override;
  void FormFieldFocusChange(PDFEngine::FocusFieldType type) override;
  bool IsPrintPreview() const override;
  SkColor GetBackgroundColor() const override;
  void SelectionChanged(const gfx::Rect& left, const gfx::Rect& right) override;
  void CaretChanged(const gfx::Rect& caret_rect) override;
  void EnteredEditMode() override;
  void DocumentFocusChanged(bool document_has_focus) override;
  void SetSelectedText(const std::string& selected_text) override;
  void SetLinkUnderCursor(const std::string& link_under_cursor) override;
  bool IsValidLink(const std::string& url) override;
```

###### pdf::mojom::PdfListener 相关接口

```
  void SetCaretPosition(const gfx::PointF& position) override;
  void MoveRangeSelectionExtent(const gfx::PointF& extent) override;
  void SetSelectionBounds(const gfx::PointF& base,
                          const gfx::PointF& extent) override;
```



###### UrlLoader::Client 相关接口

```
  bool IsValid() const override;
  blink::WebURL CompleteURL(const blink::WebString& partial_url) const override;
  net::SiteForCookies SiteForCookies() const override;
  void SetReferrerForRequest(blink::WebURLRequest& request,
                             const blink::WebURL& referrer_url) override;
  std::unique_ptr<blink::WebAssociatedURLLoader> CreateAssociatedURLLoader(
      const blink::WebAssociatedURLLoaderOptions& options) override;
```

###### PostMessageReceiver::Client

```
void OnMessage(const base::Value::Dict& message) override;
```

###### *PaintManager::Client 

```
  void InvalidatePluginContainer() override;
  void OnPaint(const std::vector<gfx::Rect>& paint_rects,
               std::vector<PaintReadyRect>& ready,
               std::vector<gfx::Rect>& pending) override;
  void UpdateSnapshot(sk_sp<SkImage> snapshot) override;
  void UpdateScale(float scale) override;
  void UpdateLayerTransform(float scale,
                            const gfx::Vector2dF& translate) override;
```

###### PdfAccessibilityActionHandler

```
  void EnableAccessibility() override;
  void HandleAccessibilityAction(
      const AccessibilityActionData& action_data) override;
  void LoadOrReloadAccessibility() override;
```

###### PdfAccessibilityImageFetcher

```
  SkBitmap GetImageForOcr(int32_t page_index,
                          int32_t page_object_index) override;
```

###### PreviewModeClient::Client

```
  void PreviewDocumentLoadComplete() override;
  void PreviewDocumentLoadFailed() override;
```

###### 私有相关接口

```
 bool InitializeForTesting();

  const gfx::Rect& GetPluginRectForTesting() const { return plugin_rect_; }

  float GetDeviceScaleForTesting() const { return device_scale_; }

  DocumentLoadState document_load_state_for_testing() const {
    return document_load_state_;
  }

  int GetContentRestrictionsForTesting() const {
    return GetContentRestrictions();
  }

  AccessibilityDocInfo GetAccessibilityDocInfoForTesting() const {
    return GetAccessibilityDocInfo();
  }

  int32_t next_accessibility_page_index_for_testing() const {
    return next_accessibility_page_index_;
  }

  void set_next_accessibility_page_index_for_testing(int32_t index) {
    next_accessibility_page_index_ = index;
  }

 private:
  // Callback that runs after `LoadUrl()`. The `loader` is the loader used to
  // load the URL, and `result` is the result code for the load.
  using LoadUrlCallback =
      base::OnceCallback<void(std::unique_ptr<UrlLoader> loader,
                              int32_t result)>;

  enum class AccessibilityState {
    kOff = 0,  // Off.
    kPending,  // Enabled but waiting for doc to load.
    kLoaded,   // Fully loaded.
  };

  struct BackgroundPart {
    gfx::Rect location;
    uint32_t color;
  };

  // Metadata about an available preview page.
  struct PreviewPageInfo {
    // Data source URL.
    std::string url;

    // Page index in destination document.
    int dest_page_index = -1;
  };

  // Call `Destroy()` instead.
  ~PdfViewWebPlugin() override;

  bool InitializeCommon();

  // Sends whether to do smooth scrolling.
  void SendSetSmoothScrolling();

  // Handles `LoadUrl()` result for the main document.
  void DidOpen(std::unique_ptr<UrlLoader> loader, int32_t result);

  // Updates the scroll position, which is in CSS pixels relative to the
  // top-left corner.
  void UpdateScroll(const gfx::PointF& scroll_position);

  // Loads `url`, invoking `callback` on receiving the initial response.
  void LoadUrl(std::string_view url, LoadUrlCallback callback);

  // Handles `Open()` result for `form_loader_`.
  void DidFormOpen(int32_t result);

  // Sends start/stop loading notifications to the plugin's render frame.
  void DidStartLoading();
  void DidStopLoading();

  // Gets the content restrictions based on the permissions which `engine_` has.
  int GetContentRestrictions() const;

  // Message handlers.
  void HandleDisplayAnnotationsMessage(const base::Value::Dict& message);
  void HandleGetNamedDestinationMessage(const base::Value::Dict& message);
  void HandleGetPageBoundingBoxMessage(const base::Value::Dict& message);
  void HandleGetPasswordCompleteMessage(const base::Value::Dict& message);
  void HandleGetSelectedTextMessage(const base::Value::Dict& message);
  void HandleGetThumbnailMessage(const base::Value::Dict& message);
  void HandlePrintMessage(const base::Value::Dict& /*message*/);
  void HandleRotateClockwiseMessage(const base::Value::Dict& /*message*/);
  void HandleRotateCounterclockwiseMessage(
      const base::Value::Dict& /*message*/);
  void HandleSaveAttachmentMessage(const base::Value::Dict& message);
  void HandleSaveMessage(const base::Value::Dict& message);
  void HandleSelectAllMessage(const base::Value::Dict& /*message*/);
  void HandleSetBackgroundColorMessage(const base::Value::Dict& message);
  void HandleSetPresentationModeMessage(const base::Value::Dict& message);
  void HandleSetTwoUpViewMessage(const base::Value::Dict& message);
  void HandleStopScrollingMessage(const base::Value::Dict& message);
  void HandleViewportMessage(const base::Value::Dict& message);

  void SaveToBuffer(const std::string& token);
  void SaveToFile(const std::string& token);

  // Converts a scroll offset (which is relative to a UI direction-dependent
  // scroll origin) to a scroll position (which is always relative to the
  // top-left corner).
  gfx::PointF GetScrollPositionFromOffset(
      const gfx::Vector2dF& scroll_offset) const;

  // Paints the given invalid area of the plugin to the given graphics device.
  // PaintManager::Client::OnPaint() should be its only caller.
  void DoPaint(const std::vector<gfx::Rect>& paint_rects,
               std::vector<PaintReadyRect>& ready,
               std::vector<gfx::Rect>& pending);

  // The preparation when painting on the image data buffer for the first
  // time.
  void PrepareForFirstPaint(std::vector<PaintReadyRect>& ready);

  // Updates the available area and the background parts, notifies the PDF
  // engine, and updates the accessibility information.
  void OnGeometryChanged(double old_zoom, float old_device_scale);

  // A helper of OnGeometryChanged() which updates the available area and
  // the background parts, and notifies the PDF engine of geometry changes.
  void RecalculateAreas(double old_zoom, float old_device_scale);

  // Figures out the location of any background rectangles (i.e. those that
  // aren't painted by the PDF engine).
  void CalculateBackgroundParts();

  // Computes document width/height in device pixels, based on current zoom and
  // device scale
  int GetDocumentPixelWidth() const;
  int GetDocumentPixelHeight() const;

  // Schedules invalidation tasks after painting finishes.
  void InvalidateAfterPaintDone();

  // Callback to clear deferred invalidates after painting finishes.
  void ClearDeferredInvalidates();

  // Recalculates values that depend on scale factors.
  void UpdateScaledValues();

  void OnViewportChanged(const gfx::Rect& new_plugin_rect_in_css_pixel,
                         float new_device_scale);

  // Text editing methods.
  bool SelectAll();
  bool Cut();
  bool Paste(const blink::WebString& value);
  bool Undo();
  bool Redo();

  bool HandleWebInputEvent(const blink::WebInputEvent& event);

  // Helper method for converting IME text to input events.
  // TODO(crbug.com/40199248): Consider handling composition events.
  void HandleImeCommit(const blink::WebString& text);

  // Callback to print without re-entrancy issues. The callback prevents the
  // invocation of printing in the middle of an event handler, which is risky;
  // see crbug.com/66334.
  // TODO(crbug.com/40185029): Re-evaluate the need for a callback when parts of
  // the plugin are moved off the main thread.
  void OnInvokePrintDialog();

  void ResetRecentlySentFindUpdate();

  // Records metrics about the document metadata.
  void RecordDocumentMetrics();

  // Sends the attachments data.
  void SendAttachments();

  // Sends the bookmarks data.
  void SendBookmarks();

  // Send document metadata data.
  void SendMetadata();

  // Sends the loading progress, where `percentage` represents the progress, or
  // -1 for loading error.
  void SendLoadingProgress(double percentage);

  // Handles message for resetting Print Preview.
  void HandleResetPrintPreviewModeMessage(const base::Value::Dict& message);

  // Handles message for loading a preview page.
  void HandleLoadPreviewPageMessage(const base::Value::Dict& message);

  // Starts loading the next available preview page into a blank page.
  void LoadAvailablePreviewPage();

  // Handles `LoadUrl()` result for a preview page.
  void DidOpenPreview(std::unique_ptr<UrlLoader> loader, int32_t result);

  // Continues loading the next preview page.
  void LoadNextPreviewPage();

  // Sends a notification that the print preview has loaded.
  void SendPrintPreviewLoadedNotification();

  // Sends the thumbnail image data.
  void SendThumbnail(base::Value::Dict reply, Thumbnail thumbnail);

  // Converts `frame_coordinates` to PDF coordinates.
  gfx::Point FrameToPdfCoordinates(const gfx::PointF& frame_coordinates) const;

  // Gets the accessibility doc info based on the information from `engine_`.
  AccessibilityDocInfo GetAccessibilityDocInfo() const;

  // Sets the accessibility information about the given `page_index` in the
  // renderer.
  void PrepareAndSetAccessibilityPageInfo(int32_t page_index);

  // Prepares the accessibility information about the current viewport. This is
  // done once when accessibility is first loaded and again when the geometry
  // changes.
  void PrepareAndSetAccessibilityViewportInfo();

  // Starts loading accessibility information.
  void LoadAccessibility();
```



#### PaintManager

```
这个应该就是渲染的管理类， 上层通过这个管理类来管理pdf的渲染结构。
```

##### Client类

