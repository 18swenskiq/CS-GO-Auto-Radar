# Node types
Simple node definitions
## Generative:
(nodes that just makes a thing)





------------
### Texture ( `texture` )
Renders a texture into node buffer.

#### Properties:

| Prop Name | Type | Default | Description |
| ------ | ------ | ------- | ------ |
| `source` | `string` | `"textures/modulate.png"` | Source path for the texture |

#### Outputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `output` | RGBA | out |





------------

### Color ( `color` )
Clears a color into node

#### Properties:

| Prop Name | Type | Default | Description |
| ------ | ------ | ------- | ------ |
| `color` | `vec4` | `(1.0, 1.0, 1.0, 1.0)` | Colour innit |

#### Outputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `output` | RGBA | out |



------------

## Transformative:
(Takes input and makes output)






------------
### Distance ( `distance` )
Computes distance to nearest 'landmass'

#### Properties:

| Prop Name | Type | Default | Description |
| ------ | ------ | ------- | ------ |
| `maxdist` | `int` | `255` | Maxmimum iterations the node should compute |

#### Inputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `mask` | R | B/W mask which represents 'landmasses'|

#### Outputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `output` | R | Distance |







------------
### Guassian Blur ( `guassian` )
Blurs input

#### Properties:

| Prop Name | Type | Default | Description |
| ------ | ------ | ------- | ------ |
| `iterations` | `int` | `8` | Blur iterations |
| `radius` | `float ` | `10.0f` | Blur radius |

#### Inputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `image` | RGBA | Standard image input |

#### Outputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `output` | RGBA | Blurred image|






------------
### Ambient Occlusion ( `aopass` )
Computes true ambient occlusion based on GBuffer position / normals.

#### Properties:

| Prop Name | Type | Default | Description |
| ------ | ------ | ------- | ------ |
| `radius` | `float` | `256.0f` | Occlusion influence distance |
| `iterations` | `int` | `64` | AO steps per pixel (x64 samples) |
| `bias` | `float` | `1.0f` | Unit bias to consider when calculating ao |
| `accum_divisor` | `float` | `15.0f` | How quickly AO accumulates (smaller = faster) (0 = error) |
| `matrix.proj` | `mat4` | `identity` | Projection that was used when calculating the source buffers |
| `matrix.view` | `mat4` | `identity` | Camera transform that was used when calculating the source buffers |

#### Inputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `gposition` | RGBA16F | Gbuffer position buffer |
| `gnormal` | RGBA16F | Gbuffer normal buffer |

#### Outputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `output` | R | Ambient occlusion factor (255 = occluded, 0 = exposed) |





------------
### Invert ( `invert` )
Inverts image

#### Inputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `input` | RGBA | input |

#### Outputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `output` | RGBA | ! input |





------------
### Passthrough ( `passthrough` )
Copies image through node (does nothing, except waste gpu memory, basically)

#### Inputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `input` | RGBA | input |

#### Outputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `output` | RGBA | input |





------------
### Blend ( `blend` )
Blends two inputs based on mask

#### Properties:

| Prop Name | Type | Default | Description |
| ------ | ------ | ------- | ------ |
| `mode` | `enum( MIX/ADD/SUB/MUL )` | `BLEND_MIX` | Blending mode |
| `factor` | `float` | `1.0f` | Blend factor (multiplies mask value) |

#### Inputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `Base` | RGBA | Layer0 input |
| `Layer` | RGBA | Layer1 input |
| `Mask` | R | Mask input |

#### Outputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `output` | RGBA | Blended image |





------------
### Gradient Map ( `gradient` )
Colors an image based on gradient input

#### Properties:

| Prop Name | Type | Default | Description |
| ------ | ------ | ------- | ------ |
| `glGradientID` | `int` | `0` | Texture ID from OpenGL to use as the gradient texture |
| `channelID` | `int` | `0` | Channel to use when sampling the source texture |
| `min` | `float` | `0.0f` | Remap from this value when sampling source |
| `max` | `float` | `1.0f` | Remap to this value when sampling source |

#### Inputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `Layer` | RGBA | Layer0 input |

#### Outputs:

| Name | Format | Description |
| ------ | ------ | ------ |
| `output` | RGBA | Gradient applied image |