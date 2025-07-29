class WD3Widget {
  constructor(APP, widget, width, height) {
    this.svgWidth = width;
    this.svgHeight = height;
    this.D3Objs = {};
    this.svg = d3.create("svg")
      .attr("width", this.svgWidth)
      .attr("height", this.svgHeight)
      .attr("overflow", "hidden");

    widget.appendChild(this.svg.node());

    this.saveD3Obj = function(id, obj) {
      this.D3Objs[id] = obj;
    };

    this.getD3Obj = function(id) {
      return this.D3Objs[id];
    };

    this.removeD3Obj = function(id) {
      delete this.D3Objs[id];
    };

    this.body = function() {
      return this.svg;
    };
  }
}
