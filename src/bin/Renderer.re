open Revery;
open Revery.Draw;
open Revery.UI;
open Revery.UI.Components;

module TerminalView = {
    let%component make = (~model, ()) => {

    open Model;

    let (cursorLeft, cursorTop) = switch (model.font) {
    | None => (0., 0.)
    | Some({characterWidth, lineHeight, _}) => 
      let cursorLeft = float(model.cursor.column) *. characterWidth;
      let cursorTop=float(model.cursor.row) *. lineHeight;
      (cursorLeft, cursorTop);
    };

    let stiffSpring = Spring.Options.create(~stiffness=250., ~damping=20., ());

    let%hook (springyCursorLeft, _setCursorRowImmediately) =
      Hooks.spring(
        ~target=cursorLeft,
        stiffSpring,
      );
    let%hook (springyCursorTop, _setCursorTopImmediately) =
      Hooks.spring(
        ~target=cursorTop,
        stiffSpring,
      );
      
    let containerStyle =
      Style.[
        backgroundColor(Colors.black),
        position(`Absolute),
        justifyContent(`Center),
        alignItems(`Center),
        bottom(0),
        top(0),
        left(0),
        right(0),
      ];

    let innerStyle = Style.[flexDirection(`Row), alignItems(`FlexEnd)];

    let getColor = (color: Vterm.Color.t) => {
      switch (color) {
      | DefaultBackground => Colors.black |> Color.toSkia
      | DefaultForeground => Colors.purple |> Color.toSkia
      | Rgb(r, g, b) =>
        Skia.Color.makeArgb(
          255l,
          r |> Int32.of_int,
          g |> Int32.of_int,
          b |> Int32.of_int,
        )
      | _ => Colors.green |> Color.toSkia
      };
    };

    let getFgColor = (cell: Vterm.ScreenCell.t) => {
      cell.reverse == 0 ? getColor(cell.fg) : getColor(cell.bg);
    };

    let getBgColor = (cell: Vterm.ScreenCell.t) => {
      cell.reverse == 0 ? getColor(cell.bg) : getColor(cell.fg);
    };

    let element =
      <Canvas
        style=containerStyle
        render={canvasContext => {
         open Model;
          model.font
          |> Option.iter(
               (
                 {font, lineHeight, characterWidth, characterHeight, fontSize}: Msg.fontInfo,
               ) => {
               let defaultBackground = Colors.black |> Color.toSkia;

               let backgroundPaint = Skia.Paint.make();
               Skia.Paint.setAntiAlias(backgroundPaint, false);

               let textPaint = Skia.Paint.make();
               let typeFace = Revery.Font.getSkiaTypeface(font);
               Skia.Paint.setTypeface(textPaint, typeFace);
               Skia.Paint.setTextSize(textPaint, 12.);
               Skia.Paint.setAntiAlias(textPaint, true);
               Skia.Paint.setSubpixelText(textPaint, true);
               Skia.Paint.setLcdRenderText(textPaint, true);

               let columns = Screen.getColumns(model.screen);
               let rows = Screen.getRows(model.screen);
               for (column in 0 to columns - 1) {
                 for (row in 0 to rows - 1) {
                   let cell = Screen.getCell(~row, ~column, model.screen);

                   let bgColor = getBgColor(cell);
                   if (bgColor != defaultBackground) {
                     Skia.Paint.setColor(backgroundPaint, bgColor);
                     CanvasContext.drawRectLtwh(
                       ~paint=backgroundPaint,
                       ~left=float(column) *. characterWidth,
                       ~top=float(row) *. lineHeight,
                       ~height=lineHeight,
                       ~width=characterWidth,
                       canvasContext,
                     );
                   };
                 };
               };

               for (column in 0 to columns - 1) {
                 for (row in 0 to rows - 1) {
                   let cell = Screen.getCell(~row, ~column, model.screen);

                   let (fgColor, bgColor) =
                     if (cell.reverse == 0) {
                       let bgColor = getColor(cell.bg);
                       let fgColor = getColor(cell.fg);
                       (fgColor, bgColor);
                     } else {
                       let bgColor = getColor(cell.fg);
                       let fgColor = getColor(cell.bg);
                       (fgColor, bgColor);
                     };

                   Skia.Paint.setColor(textPaint, fgColor);
                   if (String.length(cell.chars) > 0) {
                     CanvasContext.drawText(
                       ~paint=textPaint,
                       ~x=float(column) *. characterWidth,
                       ~y=float(row) *. lineHeight +. characterHeight,
                       ~text=String.make(1, cell.chars.[0]),
                       canvasContext,
                     );
                   };
                 };
               };

               // If the cursor is visible, let's paint it now
               if (model.cursor.visible) {
                 Skia.Paint.setColor(textPaint, Colors.white |> Color.toSkia);
                 CanvasContext.drawRectLtwh(
                   ~paint=textPaint,
                   ~left=springyCursorLeft,
                   ~top=springyCursorTop,
                   ~width=characterWidth,
                   ~height=lineHeight,
                   canvasContext,
                 );
               };
             })
        }}
      />;

    element;
  
    };
};

let render = (model: Model.t) => <TerminalView model />;
