WT_JS(
var createFilter = function(upload, chunkSize) {
  var position = 0;
  
  return function(sendChunkFn) {
    var fileReader = new FileReader();
    fileReader.readAsBinaryString(upload.file.slice(position, position + chunkSize));
    
    fileReader.onload = function() {
      position += this.result.length;
      last = (position >= upload.file.size);

      
      var compressedData = pako.gzip(this.result);
      var blob = new Blob([compressedData]);
      
      
      chunk = new Object();
      chunk.upload = upload;
      chunk.data = blob; /*new Blob([this.result]);*/
      chunk.last = last;
      sendChunkFn(chunk);
    };
  };
}
)
