// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_INK_INK_STROKE_H_
#define PDF_INK_INK_STROKE_H_

namespace chrome_pdf {

class InkModeledShapeView;
class InkStrokeInputBatchView;

class InkStroke {
 public:
  virtual ~InkStroke() = default;

  virtual const InkStrokeInputBatchView& GetInputs() const = 0;

  virtual const InkModeledShapeView& GetShape() const = 0;
};

}  // namespace chrome_pdf

#endif  // PDF_INK_INK_STROKE_H_
