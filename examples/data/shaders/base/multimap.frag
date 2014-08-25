uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D illumTexture;
uniform sampler2D normalTexture;
uniform float d3d_SceneLuminance = 1.0;

varying vec3 vNormal;
varying vec3 vLightDir;
varying vec3 vLightDir2;
varying vec3 vPos;
varying vec3 vCamera;
varying vec3 vViewDir;



struct FragParams
{
   vec3 pos;
   vec2 uv;
   vec4 color; // varied from vertex shader
   vec3 normal; // varied from vertex shader
   vec3 worldNormal;
   vec3 viewDir;
   vec3 cameraPos;
   mat3 tbn;
   float sceneLuminance;
};

struct EffectParams
{
   vec4 colorContrib;
   vec3 lightContrib;
   vec4 specContrib;
   vec4 envContrib;
   vec4 illumContrib; // self-glow
   vec4 fogContrib;
};

struct MapParams
{
   vec4 diffuse;
   vec4 normal;
   vec4 specular;
   vec4 roughness;
   vec4 illum;
   vec4 irradiance;
};



// External Functions
vec4 computeMultiMapColor(MapParams m, out FragParams f, out EffectParams e);



vec4 combineEffects(EffectParams e)
{
   vec4 result = e.colorContrib;
   result.rgb *= e.lightContrib.rgb;
   result.rgb += e.envContrib.rgb;
   result.rgb += e.specContrib.rgb;
   result.rgb += e.illumContrib.rgb;
   return result;
}

void main(void)
{
   vec4 zeroVec = vec4(0,0,0,0);
   vec2 uv = gl_TexCoord[0].xy;
   
   FragParams f;
   f.uv = uv;
   f.pos = vPos;
   f.normal = vNormal;
   f.viewDir = vViewDir;
   f.cameraPos = vCamera;
   f.color = gl_Color;
   f.sceneLuminance = d3d_SceneLuminance;
   
   EffectParams e;
   e.lightContrib = zeroVec.rgb;
   e.specContrib = zeroVec;
   e.envContrib = zeroVec;
   e.fogContrib = zeroVec;
   e.illumContrib = zeroVec;

 
   MapParams m;
   m.diffuse = texture2D(diffuseTexture, uv);
   m.specular.rgb = texture2D(specularTexture, uv).rgb;
   m.specular.a = 1.0;
   m.illum.rgb = texture2D(illumTexture, uv).rgb;
   m.normal.rgb = normalize(texture2D(normalTexture, uv).rgb);
   m.irradiance = vec4(0,0,0,0);
   
   vec4 result = computeMultiMapColor(m, f, e);
   
   //gl_FragColor = result;
   
   // DEBUG:
   /*vec3 oneVec = vec3(1,1,1);
   vec3 tan = (f.tbn[0] + oneVec)*0.5;
   vec3 bitan = (f.tbn[1] + oneVec)*0.5;
   vec3 norm = (f.tbn[2] + oneVec)*0.5;
   vec3 worldNorm = (f.worldNormal + oneVec)*0.5;
   vec3 normVaried = (vNormal + oneVec)*0.5;*/
   //gl_FragColor = vec4(e.specContrib.rgb,1.0);
   //gl_FragColor = vec4(e.specContrib.rgb * m.diffuse.rgb,1.0);
   //gl_FragColor = vec4(e.lightContrib.rgb,1.0);
   //gl_FragColor = vec4((e.lightContrib.rgb + f.envContrib.rgb) * m.diffuse.rgb,1.0);
   //gl_FragColor = vec4(e.lightContrib.rgb * m.diffuse.rgb,1.0);
   //gl_FragColor = vec4(e.envContrib.rgb + m.diffuse.rgb,1.0);
   //gl_FragColor = vec4(e.envContrib.rgb,1.0);
   //gl_FragColor = vec4(e.illumContrib.rgb,1.0);
   //gl_FragColor = vec4(tan,1.0);
   //gl_FragColor = vec4(bitan,1.0);
   //gl_FragColor = vec4(norm,1.0);
   //gl_FragColor = vec4(worldNorm,1.0);
   //gl_FragColor = vec4(worldNorm.rrr,1.0);
   //gl_FragColor = vec4(normVaried,1.0);
   //gl_FragColor = vec4(gl_Color.rgb,1.0);
   
   result = combineEffects(e);
   gl_FragColor = result;
}