//====================================================================
// RESOURCE TO BYTE ARRAY LOADER
//Deprecated
function load_binary_resource(url) {
  var byteArray = [];
  var req = new XMLHttpRequest();
  req.open('GET', url, false);
  req.overrideMimeType('text\/plain; charset=x-user-defined');
  req.send(null);
  if (req.status != 200) return byteArray;
  for (var i = 0; i < req.responseText.length; ++i) {
  byteArray.push(req.responseText.charCodeAt(i) & 0xff)
  }
  return byteArray;
}


//=========================================================
// BINARY READER class

//Takes in a byte array as source data
//Works like fstream from c++
function binaryReader(bytes)
{
  this.pos = 0;
  this.data = bytes;

  this.readUint32 = function()
  {
    var val = (this.data[this.pos + 3] << 24) +
              (this.data[this.pos + 2] << 16) +
              (this.data[this.pos + 1] << 8) +
              this.data[this.pos + 0];
    this.pos += 4;
    return val;
  }

  this.readString = function()
  {
    var val = "";
    while(true)
    {
      if(this.data[this.pos] != 0x0)
      {
        val += String.fromCharCode(this.data[this.pos]);
        this.pos++;
      }
      else
      {
        this.pos++;
        break;
      }
    }

    return val;
  }

  this.readFloat = function()
  {
    var dat = [this.data[this.pos+3],
    this.data[this.pos+2],
    this.data[this.pos+1],
    this.data[this.pos+0]];
    var buf = new ArrayBuffer(4);
    var view = new DataView(buf);

    dat.forEach(function (b, i) {
      view.setUint8(i, b);
    });

    var val = view.getFloat32(0);

    this.pos += 4;
    return val;
  }
}
