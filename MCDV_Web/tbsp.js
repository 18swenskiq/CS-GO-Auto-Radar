//===================================================
// STRUCTURES
function mesh()
{
  this.positions = [];
  this.normals = [];
  this.uvCoords = [];
  this.indices = [];
}

function info_model()
{
  this.position = vec3.create();
  this.rotation = vec3.create();
  this.modelID = 0;
}

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

//====================================================================
// TMDL class
function tbsp_model(bytes)
{
  var reader = new binaryReader(bytes);

  //Read magic number
  var magicNum = reader.readUint32();
  var version = reader.readUint32();

  var vertCount = reader.readUint32();
  var indexCount = reader.readUint32();

  this.internal_mesh = new mesh();

  //Read vertex data
  for(var c_vert = 0; c_vert < vertCount; c_vert++)
  {
    this.internal_mesh.positions.push(reader.readFloat()); //v_position
    this.internal_mesh.positions.push(reader.readFloat());
    this.internal_mesh.positions.push(reader.readFloat());

    this.internal_mesh.normals.push(reader.readFloat()); //v_normal
    this.internal_mesh.normals.push(reader.readFloat());
    this.internal_mesh.normals.push(reader.readFloat());

    this.internal_mesh.uvCoords.push(reader.readFloat()); //v_texCoord
    this.internal_mesh.uvCoords.push(reader.readFloat());
  }

  //Read indices data
  for(var c_index = 0; c_index < indexCount; c_index++)
  {
    this.internal_mesh.indices.push(reader.readUint32());
  }
}

//================================================================
// TBSP Level class

function tbsp_level(bytes)
{
  var reader = new binaryReader(bytes);

  //===========================================
  // Read in the header
  var magicNum = reader.readUint32();
  var version = reader.readUint32();

  var meshCount = reader.readUint32();
  var meshLocation = reader.readUint32();

  var modelCount = reader.readUint32();
  var modelLocation = reader.readUint32();

  var modelDictCount = reader.readUint32();
  var modelDictLocation = reader.readUint32();

  var entityCount = reader.readUint32();
  var entityLocation = reader.readUint32();

//Print some debug info if set
if(false)
{
  console.log("Reading TBSP level. Info:");
  console.log("Magic num: " + magicNum);
  console.log("Ver: " + version);
  console.log("mesh count: " + meshCount);
  console.log("Mesh location: " + meshLocation);
  console.log("Model count: " + modelCount);
  console.log("Model location: " + modelLocation);
  console.log("Model dict size: " + modelDictCount);
  console.log("Model dict location: " + modelDictLocation);
  console.log("Entity count: " + entityCount);
  console.log("Entity location: " + entityLocation);
}

  //====================================================
  // READ BSP MESHES
  reader.pos = meshLocation;
  this.meshes = [];
  for(var c_mesh = 0; c_mesh < meshCount; c_mesh++)
  {
    var h_vert_count = reader.readUint32();
    var h_indices_count = reader.readUint32();

    var c_mesh_obj = new mesh();


    for(var c_vert = 0; c_vert < h_vert_count; c_vert++)
    {
      c_mesh_obj.positions.push(reader.readFloat());
      c_mesh_obj.positions.push(reader.readFloat());
      c_mesh_obj.positions.push(reader.readFloat());

      c_mesh_obj.normals.push(reader.readFloat());
      c_mesh_obj.normals.push(reader.readFloat());
      c_mesh_obj.normals.push(reader.readFloat());

      c_mesh_obj.uvCoords.push(reader.readFloat());
      c_mesh_obj.uvCoords.push(reader.readFloat());
    }

    for(var c_index = 0; c_index < h_indices_count; c_index++)
    {
      c_mesh_obj.indices.push(reader.readUint32());
    }

    this.meshes.push(c_mesh_obj);
  }

  reader.pos = modelDictLocation;
  this.cached_models = [];

  for(var c_mdl = 0; c_mdl < modelDictCount; c_mdl++)
  {
    //Read the model String
    var mdlStr = reader.readString();

    //Load the model data
    var mdlData = load_binary_resource("ar_baggage.resources/" + mdlStr.replace(".mdl", ".tmdl"));
    var model = new tbsp_model(mdlData);

    this.cached_models.push(model);
  }

  reader.pos = modelLocation;
  this.models = [];

  for(var c_model = 0; c_model < modelCount; c_model++)
  {
    var model = new info_model();

    //Read model positions
    var m_v1 = reader.readFloat();
    var m_v2 = reader.readFloat();
    var m_v3 = reader.readFloat();

    model.position[0] = m_v1;
    model.position[1] = m_v2;
    model.position[2] = m_v3;

    var r_v1 = reader.readFloat();
    var r_v2 = reader.readFloat();
    var r_v3 = reader.readFloat();

    model.rotation[0] = r_v1;
    model.rotation[1] = r_v2;
    model.rotation[2] = r_v3;

    model.modelID = reader.readUint32();

    this.models.push(model);
  }
}
