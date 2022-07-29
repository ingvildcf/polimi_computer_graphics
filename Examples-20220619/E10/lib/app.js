var canvas;

var canw, canh;

var gl = null,
	program = null,
	worldMesh = null,
	imgtx = null;
	
var projectionMatrix, 
	perspectiveMatrix,
	viewMatrix,
	worldMatrix;
var extView = 1;
var worldScale;


//Parameters for Camera
var cx = 0.0;
var cy = 0.0;
var cz = 6.5;
var elevation = 0.01;
var angle = 0.01;
var roll = 0.01;
var modRot = 0.0;
var EVelevation = -15;
var EVangle = 30;

var lookRadius = 10.0;


var keys = [];
var rvx = 0.0;
var rvy = 0.0;
var rvz = 0.0;

var NvertexBuffer;
var NnormalBuffer;
var NtextureBuffer;
var NindexBuffer;


var keyFunctionDown =function(e) {
  if(!keys[e.keyCode]) {
  	keys[e.keyCode] = true;
	switch(e.keyCode) {
	  case 37:
//console.log("KeyUp   - Dir LEFT");
		rvy = rvy - 1.0;
		break;
	  case 39:
//console.log("KeyUp   - Dir RIGHT");
		rvy = rvy + 1.0;
		break;
	  case 38:
//console.log("KeyUp   - Dir UP");
		rvx = rvx + 1.0;
		break;
	  case 40:
//console.log("KeyUp   - Dir DOWN");
		rvx = rvx - 1.0;
		break;
	}
  }
}

var keyFunctionUp =function(e) {
  if(keys[e.keyCode]) {
  	keys[e.keyCode] = false;
	switch(e.keyCode) {
	  case 37:
//console.log("KeyDown  - Dir LEFT");
		rvy = rvy + 1.0;
		break;
	  case 39:
//console.log("KeyDown - Dir RIGHT");
		rvy = rvy - 1.0;
		break;
	  case 38:
//console.log("KeyDown - Dir UP");
		rvx = rvx - 1.0;
		break;
	  case 40:
//console.log("KeyDown - Dir DOWN");
		rvx = rvx + 1.0;
		break;
	  case 81:
	}
  }
}
function doResize() {
    // set canvas dimensions
	var canvas = document.getElementById("my-canvas");
    if((window.innerWidth > 40) && (window.innerHeight > 360)) {
		canvas.width  = window.innerWidth-16;
		canvas.height = window.innerHeight-320;
		canw = canvas.clientWidth;
		canh = canvas.clientHeight;
		
		gl.clearColor(0.0, 0.0, 0.0, 1.0);
//		gl.viewport(0.0, 0.0, w, h);
		gl.clear(gl.COLOR_BUFFER_BIT);
		
//		perspectiveMatrix = utils.MakePerspective(60, w/h, 0.1, 1000.0);

    }
}

		
// Vertex shader
var vs = `#version 300 es
#define POSITION_LOCATION 0
#define NORMAL_LOCATION 1
#define UV_LOCATION 2

layout(location = POSITION_LOCATION) in vec3 in_pos;
layout(location = NORMAL_LOCATION) in vec3 in_norm;
layout(location = UV_LOCATION) in vec2 in_uv;

uniform mat4 pMatrix;
uniform mat4 wMatrix;

out vec3 fs_pos;
out vec3 fs_norm;
out vec2 fs_uv;

void main() {
	fs_pos = (wMatrix * vec4(in_pos, 1.0)).xyz;
	fs_norm = (wMatrix * vec4(in_norm, 0.0)).xyz;
	fs_uv = vec2(in_uv.x, 1.0-in_uv.y);
	
	gl_Position = pMatrix * vec4(in_pos, 1.0);
}`;

// Fragment shader
var fs = `#version 300 es
precision highp float;

in vec3 fs_pos;
in vec3 fs_norm;
in vec2 fs_uv;

uniform sampler2D u_texture;
uniform vec3 eyePos;

uniform vec4 ambientType;
uniform vec4 diffuseType;
uniform vec4 specularType;
uniform vec4 emissionType;

uniform vec4 LAlightType;
uniform vec3 LAPos;
uniform vec3 LADir;
uniform float LAConeOut;
uniform float LAConeIn;
uniform float LADecay;
uniform float LATarget;
uniform vec4 LAlightColor;

uniform vec4 LBlightType;
uniform vec3 LBPos;
uniform vec3 LBDir;
uniform float LBConeOut;
uniform float LBConeIn;
uniform float LBDecay;
uniform float LBTarget;
uniform vec4 LBlightColor;

uniform vec4 LClightType;
uniform vec3 LCPos;
uniform vec3 LCDir;
uniform float LCConeOut;
uniform float LCConeIn;
uniform float LCDecay;
uniform float LCTarget;
uniform vec4 LClightColor;

uniform vec4 ambientLightColor;
uniform vec4 ambientLightLowColor;
uniform vec4 SHLeftLightColor;
uniform vec4 SHRightLightColor;
uniform vec3 ADir;
uniform vec4 diffuseColor;
uniform float DTexMix;
uniform vec4 specularColor;
uniform float SpecShine;
uniform float DToonTh;
uniform float SToonTh;
uniform vec4 ambientMatColor;
uniform vec4 emitColor;
uniform float SspecKwAng;

out vec4 color;

vec3 compLightDir(vec3 LPos, vec3 LDir, vec4 lightType) {
	//lights
	// -> Point
	vec3 pointLightDir = normalize(LPos - fs_pos);
	// -> Direct
	vec3 directLightDir = LDir;
	// -> Spot
	vec3 spotLightDir = normalize(LPos - fs_pos);

	return            directLightDir * lightType.x +
					  pointLightDir * lightType.y +
					  spotLightDir * lightType.z;
}

vec4 compLightColor(vec4 lightColor, float LTarget, float LDecay, vec3 LPos, vec3 LDir,
					float LConeOut, float LConeIn, vec4 lightType) {
	float LCosOut = cos(radians(LConeOut / 2.0));
	float LCosIn = cos(radians(LConeOut * LConeIn / 2.0));

	//lights
	// -> Point
	vec4 pointLightCol = lightColor * pow(LTarget / length(LPos - fs_pos), LDecay);
	// -> Direct
	vec4 directLightCol = lightColor;
	// -> Spot
	vec3 spotLightDir = normalize(LPos - fs_pos);
	float CosAngle = dot(spotLightDir, LDir);
	vec4 spotLightCol = lightColor * pow(LTarget / length(LPos - fs_pos), LDecay) *
						clamp((CosAngle - LCosOut) / (LCosIn - LCosOut), 0.0, 1.0);
	// ----> Select final component
	return          directLightCol * lightType.x +
					pointLightCol * lightType.y +
					spotLightCol * lightType.z;
}

vec4 compDiffuse(vec3 lightDir, vec4 lightCol, vec3 normalVec, vec4 diffColor, vec3 eyedirVec) {
	// Diffuse
	float LdotN = max(0.0, dot(normalVec, lightDir));
	vec4 LDcol = lightCol * diffColor;
	// --> Lambert
	vec4 diffuseLambert = LDcol * LdotN;
	// --> Toon
	vec4 diffuseToon = max(sign(LdotN- DToonTh),0.0) * LDcol;
	// --> Oren-Nayar
	float VdotN = max(0.0, dot(normalVec, eyedirVec));
	float theta_i = acos(LdotN);
	float theta_r = acos(VdotN);
	float alpha = max(theta_i, theta_r);
	float beta = min(min(theta_i, theta_r), 1.57);
	float sigma2 = DToonTh * DToonTh * 2.46;
	float A = 1.0 - 0.5 * sigma2 / (sigma2 + 0.33);
	float B = 0.45 * sigma2 / (sigma2 + 0.09);
	vec3 v_i = normalize(lightDir - normalVec * LdotN);
	vec3 v_r = normalize(eyedirVec - normalVec * VdotN);
	float G = max(0.0, dot(v_i, v_r));
	vec4 diffuseOrenNayar = diffuseLambert * (A + B * G * sin(alpha) * tan(beta));
	// ----> Select final component
	return         diffuseLambert * diffuseType.x +
				   diffuseToon * diffuseType.y +
				   diffuseOrenNayar * diffuseType.z;
}

vec4 compSpecular(vec3 lightDir, vec4 lightCol, vec3 normalVec, vec3 eyedirVec, vec3 t, vec3 b) {
	// Specular
	float LdotN = max(0.0, dot(normalVec, lightDir));
	vec3 reflection = -reflect(lightDir, normalVec);
	float LdotR = max(dot(reflection, eyedirVec), 0.0);
	vec3 halfVec = normalize(lightDir + eyedirVec);
	float HdotN = max(dot(normalVec, halfVec), 0.0);
	float HdotT = dot(t, halfVec);
	float HdotB = dot(b, halfVec);
	
	vec4 LScol = lightCol * specularColor * max(sign(LdotN),0.0);
	// --> Phong
	vec4 specularPhong = LScol * pow(LdotR, SpecShine);
	// --> Blinn
	vec4 specularBlinn = LScol * pow(HdotN, SpecShine);
	// --> Toon Phong
	vec4 specularToonP = max(sign(LdotR - SToonTh), 0.0) * LScol;
	// --> Toon Blinn
	vec4 specularToonB = max(sign(HdotN - SToonTh), 0.0) * LScol;
	
	// --> Cook-Torrance
	LdotN = max(0.00001, LdotN);
	float VdotN = max(0.00001, dot(normalVec, eyedirVec));
	HdotN = max(0.00001, HdotN);
	float HdotV = max(0.00001, dot(halfVec, eyedirVec));
	float Gm = min(1.0, 2.0 * HdotN * min(VdotN, LdotN) / HdotV);
	float F = SToonTh + (1.0 - SToonTh) * pow(1.0 - HdotV, 5.0);
	float HtoN2 = HdotN * HdotN;
	float M = (200.0 - SpecShine) / 200.0;
	float M2 = M * M;
	float Ds = exp(- (1.0-HtoN2) / (HtoN2 * M2)) / (3.14159 * M2 * HtoN2 * HtoN2);
	float GGXk = (M+1.0)*(M+1.0)/8.0;
	float GGGX = VdotN * LdotN / (((1.0-GGXk) * VdotN + GGXk)*((1.0-GGXk) * LdotN + GGXk));
	float DGGXn = M2 * M2;
	float DGGXd = HtoN2*(M2 * M2-1.0)+1.0;
	DGGXd = 3.14 * DGGXd * DGGXd;
	float DGGX = DGGXn / DGGXd;
	
	float DG = specularType.z * GGGX * DGGX + (1.0 - specularType.z) * Gm * Ds;
	
	vec4 specularCookTorrance = LScol * F * DG / (4.0 * VdotN);
	
	// --> Ward
	float alphaX = M2;
	float alphaY = M2 * (1.0 - 0.999*SToonTh);
	float sang = sin(3.14 * SspecKwAng);
	float cang = cos(3.14 * SspecKwAng);
	float wsX = pow(HdotT * cang - HdotB * sang, 2.0);
	float wsY = pow(HdotB * cang + HdotT * sang, 2.0);

	vec4 specularWard = LScol / (12.566*sqrt(VdotN / LdotN*alphaX*alphaY)) * exp(-(wsX / alphaX + wsY / alphaY) / pow(HdotN,2.0)) ;

	// ----> Select final component
	return          specularPhong * specularType.x * (1.0 - specularType.z) * (1.0 - specularType.w) +
					specularBlinn * specularType.y * (1.0 - specularType.z) * (1.0 - specularType.w) +
					specularToonP * specularType.z * specularType.x * (1.0 - specularType.w) +
					specularToonB * specularType.z * specularType.y * (1.0 - specularType.w)+
					specularCookTorrance * specularType.w * specularType.x +
					specularWard * specularType.w * specularType.y;
}

vec4 compAmbient(vec4 ambColor, vec3 normalVec) {
	// Ambient
	// --> Ambient
	vec4 ambientAmbient = ambientLightColor * ambColor;
	// --> Hemispheric
	float amBlend = (dot(normalVec, ADir) + 1.0) / 2.0;
	vec4 ambientHemi = (ambientLightColor * amBlend + ambientLightLowColor * (1.0 - amBlend)) * ambColor;
	// --> Spherical Harmonics
	const mat4 McInv = mat4(vec4(0.25,0.0,-0.25,0.7071),vec4(0.25,0.6124,-0.25,-0.3536),vec4(0.25,-0.6124,-0.25,-0.3536),vec4(0.25,0.0,0.75,0.0));
	mat4 InCols = transpose(mat4(ambientLightLowColor, SHRightLightColor, SHLeftLightColor, ambientLightColor));
	mat4 OutCols = McInv * InCols;
	vec4 ambientSH = vec4((vec4(1,normalVec) * OutCols).rgb, 1.0) * ambColor;

	// ----> Select final component
	return 		   ambientAmbient * ambientType.x +
				   ambientHemi    * ambientType.y +
				   ambientSH      * ambientType.z;
}

void main() {
	vec4 texcol = texture(u_texture, fs_uv);
	vec4 diffColor = diffuseColor * (1.0-DTexMix) + texcol * DTexMix;
	vec4 ambColor = ambientMatColor * (1.0-DTexMix) + texcol * DTexMix;
	vec4 emit = (emitColor * (1.0-DTexMix) +
				    texcol * DTexMix * 
				   			max(max(emitColor.r, emitColor.g), emitColor.b)) * emissionType.r;
//				   		   (0.299*emitColor.r + 0.587*emitColor.g + 0.114*emitColor.b);
	
	vec3 normalVec = normalize(fs_norm);
	vec3 eyedirVec = normalize(eyePos - fs_pos);

	//// online computation of tangent and bitangent

	// compute initial tangent and bi-tangent
	float tbf = max(0.0, sign(abs(normalVec.y) - 0.707));
//	vec3 t = vec3(1,0,0) * tbf + vec3(0,1,0) * (1.0-tbf);
//	t = normalize(cross(normalVec, t));
	vec3 t = normalize(cross(normalVec, vec3(1,0,0)));
	vec3 b = normalize(cross(normalVec, t));
	
	//lights
	vec3 LAlightDir = compLightDir(LAPos, LADir, LAlightType);
	vec4 LAlightCol = compLightColor(LAlightColor, LATarget, LADecay, LAPos, LADir,
								     LAConeOut, LAConeIn, LAlightType);
	
	vec3 LBlightDir = compLightDir(LBPos, LBDir, LBlightType);
	vec4 LBlightCol = compLightColor(LBlightColor, LBTarget, LBDecay, LBPos, LBDir,
								     LBConeOut, LBConeIn, LBlightType);
	
	vec3 LClightDir = compLightDir(LCPos, LCDir, LClightType);
	vec4 LClightCol = compLightColor(LClightColor, LCTarget, LCDecay, LCPos, LCDir,
								     LCConeOut, LCConeIn, LClightType);
	
	
	// Diffuse
	vec4 diffuse = compDiffuse(LAlightDir, LAlightCol, normalVec, diffColor, eyedirVec) + 
				   compDiffuse(LBlightDir, LBlightCol, normalVec, diffColor, eyedirVec) +
				   compDiffuse(LClightDir, LClightCol, normalVec, diffColor, eyedirVec);
	
	// Specular
	vec4 specular = compSpecular(LAlightDir, LAlightCol, normalVec, eyedirVec, t, b) +
					compSpecular(LBlightDir, LBlightCol, normalVec, eyedirVec, t, b) +
					compSpecular(LClightDir, LClightCol, normalVec, eyedirVec, t, b);
	
	// Ambient
	vec4 ambient = compAmbient(ambColor, normalVec);
	
	// final steps
	float sctf = (SspecKwAng - 1.0) * (specularType.w * specularType.x) + 1.0;
	float dctf = 1.0 - SspecKwAng* (specularType.w * specularType.x);
	vec4 out_color = clamp(ambient + dctf*diffuse + sctf*specular + emit, 0.0, 1.0);
	
	color = vec4(out_color.rgb, 1.0);
}`;

// event handler

var mouseState = false;
var lastMouseX = -100, lastMouseY = -100;
function doMouseDown(event) {
	lastMouseX = event.pageX;
	lastMouseY = event.pageY;
	mouseState = true;
}
function doMouseUp(event) {
	lastMouseX = -100;
	lastMouseY = -100;
	mouseState = false;
}
function doMouseMove(event) {
	if(mouseState) {
	 var dx = event.pageX - lastMouseX;
	var dy = lastMouseY - event.pageY;
	 if((event.pageX <= 0.66 * canw) || (extView == 0)) {		
		if((dx != 0) || (dy != 0)) {
			angle += 0.5 * dx;
			elevation += 0.5 * dy;
		}
	  } else {
		if((dx != 0) || (dy != 0)) {
			EVangle += 0.5 * dx;
			EVelevation += 0.5 * dy;
		}
//		modRot = modRot + 0.5 * dx;
	  }
	  lastMouseX = event.pageX;
	  lastMouseY = event.pageY;
	}
}
function doMouseWheel(event) {
	var nLookRadius = lookRadius + event.wheelDelta/1000.0;
	if((nLookRadius > 2.0) && (nLookRadius < 20.0)) {
		lookRadius = nLookRadius;
	}
}


// UI helper arrays

UIonOff = {
	LAlightType:{
		none:{
			LA13:false, LA14:false,
			LA21:false, LA22:false, LA23:false, LA24:false,
			LA31:false, LA32:false, LA33:false, LA34:false,
			LA41:false, LA42:false, LA43:false, LA44:false,
			LA51:false, LA52:false, LA53:false, LA54:false,
			LA61:false, LA62:false
		},
		direct:{
			LA13:true, LA14:true,
			LA21:false, LA22:false, LA23:false, LA24:false,
			LA31:false, LA32:false, LA33:false, LA34:false,
			LA41:false, LA42:false, LA43:false, LA44:false,
			LA51:true, LA52:true, LA53:false, LA54:false,
			LA61:true, LA62:true
		},
		point:{
			LA13:true, LA14:true,
			LA21:true, LA22:true, LA23:true, LA24:true,
			LA31:true, LA32:true, LA33:true, LA34:true,
			LA41:true, LA42:true, LA43:false, LA44:false,
			LA51:false, LA52:false, LA53:false, LA54:false,
			LA61:false, LA62:false
		},
		spot:{
			LA13:true, LA14:true,
			LA21:true, LA22:true, LA23:true, LA24:true,
			LA31:true, LA32:true, LA33:true, LA34:true,
			LA41:true, LA42:true, LA43:true, LA44:true,
			LA51:true, LA52:true, LA53:true, LA54:true,
			LA61:true, LA62:true
		}
	},
	LBlightType:{
		none:{
			LB13:false, LB14:false,
			LB21:false, LB22:false, LB23:false, LB24:false,
			LB31:false, LB32:false, LB33:false, LB34:false,
			LB41:false, LB42:false, LB43:false, LB44:false,
			LB51:false, LB52:false, LB53:false, LB54:false,
			LB61:false, LB62:false
		},
		direct:{
			LB13:true, LB14:true,
			LB21:false, LB22:false, LB23:false, LB24:false,
			LB31:false, LB32:false, LB33:false, LB34:false,
			LB41:false, LB42:false, LB43:false, LB44:false,
			LB51:true, LB52:true, LB53:false, LB54:false,
			LB61:true, LB62:true
		},
		point:{
			LB13:true, LB14:true,
			LB21:true, LB22:true, LB23:true, LB24:true,
			LB31:true, LB32:true, LB33:true, LB34:true,
			LB41:true, LB42:true, LB43:false, LB44:false,
			LB51:false, LB52:false, LB53:false, LB54:false,
			LB61:false, LB62:false
		},
		spot:{
			LB13:true, LB14:true,
			LB21:true, LB22:true, LB23:true, LB24:true,
			LB31:true, LB32:true, LB33:true, LB34:true,
			LB41:true, LB42:true, LB43:true, LB44:true,
			LB51:true, LB52:true, LB53:true, LB54:true,
			LB61:true, LB62:true
		}
	},
	LClightType:{
		none:{
			LC13:false, LC14:false,
			LC21:false, LC22:false, LC23:false, LC24:false,
			LC31:false, LC32:false, LC33:false, LC34:false,
			LC41:false, LC42:false, LC43:false, LC44:false,
			LC51:false, LC52:false, LC53:false, LC54:false,
			LC61:false, LC62:false
		},
		direct:{
			LC13:true, LC14:true,
			LC21:false, LC22:false, LC23:false, LC24:false,
			LC31:false, LC32:false, LC33:false, LC34:false,
			LC41:false, LC42:false, LC43:false, LC44:false,
			LC51:true, LC52:true, LC53:false, LC54:false,
			LC61:true, LC62:true
		},
		point:{
			LC13:true, LC14:true,
			LC21:true, LC22:true, LC23:true, LC24:true,
			LC31:true, LC32:true, LC33:true, LC34:true,
			LC41:true, LC42:true, LC43:false, LC44:false,
			LC51:false, LC52:false, LC53:false, LC54:false,
			LC61:false, LC62:false
		},
		spot:{
			LC13:true, LC14:true,
			LC21:true, LC22:true, LC23:true, LC24:true,
			LC31:true, LC32:true, LC33:true, LC34:true,
			LC41:true, LC42:true, LC43:true, LC44:true,
			LC51:true, LC52:true, LC53:true, LC54:true,
			LC61:true, LC62:true
		}
	},
	ambientType:{
		none:{
			A20:false, A21:false, A22:false,
			A30:false, A31:false, A32:false, A33:false,
			A40:false, A41:false, A42:false, A43:false,
			A50:false, A51:false, A52:false,
			MA1:false, MA2:false
		},
		ambient:{
			A20:false, A21:true, A22:true,
			A30:false, A31:false, A32:false, A33:false,
			A40:false, A41:false, A42:false, A43:false,
			A50:false, A51:false, A52:false,
			MA1:true, MA2:true
		},
		hemispheric:{
			A20:true,  A21:false, A22:true,
			A30:false, A31:true, A32:true, A33:false,
			A40:false, A41:true, A42:true, A43:false,
			A50:false, A51:true, A52:true,
			MA1:true, MA2:true
		},
		SH:{
			A20:true,  A21:false, A22:true,
			A30:true, A31:false, A32:false, A33:true,
			A40:true, A41:false, A42:false, A43:true,
			A50:true, A51:false, A52:true,
			MA1:true, MA2:true
		}
	},
	diffuseType:{
		none:{
			D21:false, D22:false,
			D41:false, D42:false, D41b:false
		},
		lambert:{
			D21:true,  D22:true,
			D41:false, D42:false, D41b:false
		},
		toon:{
			D21:true,  D22:true,
			D41:true, D42:true, D41b:false
		},
		orenNayar:{
			D21:true,  D22:true,
			D41:false, D42:true, D41b:true
		}
	},
	specularType:{
		none:{
			S21:false, S22:false,
			S31:false, S32:false, S31b:false,
			S41:false, S42:false, S41b:false, S41c:false,
			S51a:false,S51b:false,S52:false
		},
		phong:{
			S21:true, S22:true,
			S31:true, S32:true, S31b:false,
			S41:false, S42:false, S41b:false, S41c:false,
			S51a:false,S51b:false,S52:false
		},
		blinn:{
			S21:true, S22:true,
			S31:true, S32:true, S31b:false,
			S41:false, S42:false, S41b:false, S41c:false,
			S51a:false,S51b:false,S52:false
		},
		toonP:{
			S21:true, S22:true,
			S31:false, S32:false,  S31b:false,
			S41:true, S42:true, S41b:false, S41c:false,
			S51a:false,S51b:false,S52:false
		},
		toonB:{
			S21:true, S22:true,
			S31:false, S32:false, S31b:false,
			S41:true, S42:true, S41b:false, S41c:false,
			S51a:false,S51b:false,S52:false
		},
		cookTorrance:{
			S21:true, S22:true,
			S31:false, S32:true, S31b:true,
			S41:false, S42:true, S41b:true, S41c:false,
			S51a:true,S51b:false,S52:true
		},
		cookTorranceGGX:{
			S21:true, S22:true,
			S31:false, S32:true, S31b:true,
			S41:false, S42:true, S41b:true, S41c:false,
			S51a:true,S51b:false,S52:true
		},
		ward:{
			S21:true, S22:true,
			S31:false, S32:true, S31b:true,
			S41:false, S42:true, S41b:false, S41c:true,
			S51a:false,S51b:true,S52:true
		}
	}, 
	emissionType:{
		Yes: {ME1:true,  ME2:true},
		No:  {ME1:false, ME2:false}
	}
}


function showHideUI(tag, sel) {
	for(var name in UIonOff[tag][sel]) {
		document.getElementById(name).style.display = UIonOff[tag][sel][name] ? "block" : "none";
	}
}

function showLight(sel) {
	document.getElementById("LA").style.display = (sel == "LA") ? "block" : "none";
	document.getElementById("LB").style.display = (sel == "LB") ? "block" : "none";
	document.getElementById("LC").style.display = (sel == "LC") ? "block" : "none";
}

defShaderParams = {
	ambientType: "ambient",
	diffuseType: "lambert",
	specularType: "phong",
	ambientLightColor: "#222222",
	diffuseColor: "#00ffff",
	specularColor: "#ffffff",
	ambientLightLowColor: "#002200",
	SHLeftLightColor: "#550055",
	SHRightLightColor: "#005555",
	ambientMatColor: "#00ffff",
	emitColor: "#000000",

	LAlightType: "direct",
	LAlightColor: "#ffffff",
	LAPosX: 20,
	LAPosY: 30,
	LAPosZ: 50,
	LADirTheta: 60,
	LADirPhi: 45,
	LAConeOut: 30,
	LAConeIn: 80,
	LADecay: 0,
	LATarget: 61,

	LBlightType: "none",
	LBlightColor: "#ffffff",
	LBPosX: -40,
	LBPosY: 30,
	LBPosZ: 50,
	LBDirTheta: 60,
	LBDirPhi: 135,
	LBConeOut: 30,
	LBConeIn: 80,
	LBDecay: 0,
	LBTarget: 61,

	LClightType: "none",
	LClightColor: "#ffffff",
	LCPosX: 60,
	LCPosY: 30,
	LCPosZ: 50,
	LCDirTheta: 60,
	LCDirPhi: -45,
	LCConeOut: 30,
	LCConeIn: 80,
	LCDecay: 0,
	LCTarget: 61,

	ADirTheta: 0,
	ADirPhi: 0,
	DTexMix: 100,
	SpecShine: 100,
	DToonTh: 50,
	SToonTh: 90,
	SspecKwAng: 80,
	
	emissionType: "No"	
}

//defShaderParams.ambientType= "SH";
//defShaderParams.diffuseType= "none";
//defShaderParams.specularType= "none";
//defShaderParams.ambientLightColor= "#808080";
//defShaderParams.ambientLightLowColor= "#ff0000";
//defShaderParams.SHLeftLightColor= "#00ff00";
//defShaderParams.SHRightLightColor= "#0000ff";
//defShaderParams.ambientMatColor= "#ffffff";
//defShaderParams.DTexMix =  0;


function resetShaderParams() {
	for(var name in defShaderParams) {
		value = defShaderParams[name];
		document.getElementById(name).value = value;
		if(document.getElementById(name).type == "select-one") {
			showHideUI(name, value);
		}
	}
	
	cx = 0.0;
	cy = 0.0;
	cz = 6.5;
	elevation = 0.01;
	angle = 0.01;
	roll = 0.01;
	lookRadius = 10.0;
	
	if(gl) {
		setWorldMesh();
	}
}

function unifPar(pHTML, pGLSL, type) {
	this.pHTML = pHTML;
	this.pGLSL = pGLSL;
	this.type = type;
}

function noAutoSet(gl) {
}

function val(gl) {
	gl.uniform1f(program[this.pGLSL+"Uniform"], document.getElementById(this.pHTML).value);
}

function valD10(gl) {
	gl.uniform1f(program[this.pGLSL+"Uniform"], document.getElementById(this.pHTML).value / 10);
}

function valD100(gl) {
	gl.uniform1f(program[this.pGLSL+"Uniform"], document.getElementById(this.pHTML).value / 100);
}

function valCol(gl) {
	col = document.getElementById(this.pHTML).value.substring(1,7);
    R = parseInt(col.substring(0,2) ,16) / 255;
    G = parseInt(col.substring(2,4) ,16) / 255;
    B = parseInt(col.substring(4,6) ,16) / 255;
	gl.uniform4f(program[this.pGLSL+"Uniform"], R, G, B, 1);
}

function valVec3(gl) {
	gl.uniform3f(program[this.pGLSL+"Uniform"],
				 document.getElementById(this.pHTML+"X").value / 10,
				 document.getElementById(this.pHTML+"Y").value / 10,
				 document.getElementById(this.pHTML+"Z").value / 10);
}

function valDir(gl) {
	var t = utils.degToRad(document.getElementById(this.pHTML+"Theta").value);
	var p = utils.degToRad(document.getElementById(this.pHTML+"Phi").value);
	gl.uniform3f(program[this.pGLSL+"Uniform"],Math.sin(t)*Math.sin(p), Math.cos(t), Math.sin(t)*Math.cos(p));
}

valTypeDecoder = {
	LAlightType:{
		none: [0,0,0,0],
		direct: [1,0,0,0],
		point: [0,1,0,0],
		spot: [0,0,1,0]
	},
	LBlightType:{
		none: [0,0,0,0],
		direct: [1,0,0,0],
		point: [0,1,0,0],
		spot: [0,0,1,0]
	},
	LClightType:{
		none: [0,0,0,0],
		direct: [1,0,0,0],
		point: [0,1,0,0],
		spot: [0,0,1,0]
	},
	ambientType:{
		none: [0,0,0,0],
		ambient: [1,0,0,0],
		hemispheric: [0,1,0,0],
		SH: [0,0,1,0]
	},
	diffuseType:{
		none: [0,0,0,0],
		lambert: [1,0,0,0],
		toon: [0,1,0,0],
		orenNayar: [0,0,1,0]
	},
	specularType:{
		none:  [0,0,0,0],
		phong: [1,0,0,0],
		blinn: [0,1,0,0],
		toonP: [1,0,1,0],
		toonB: [0,1,1,0],
		ward:         	 [0,1,0,1],
		cookTorrance: 	 [1,0,0,1],
		cookTorranceGGX: [1,0,1,1]
	},
	emissionType:{
		No: [0,0,0,0],
		Yes: [1,0,0,0]
	}
}

function valType(gl) {
	var v = valTypeDecoder[this.pHTML][document.getElementById(this.pHTML).value];
	gl.uniform4f(program[this.pGLSL+"Uniform"], v[0], v[1], v[2], v[3]);
}


unifParArray =[
	new unifPar("ambientType","ambientType", valType),
	new unifPar("diffuseType","diffuseType", valType),
	new unifPar("specularType","specularType", valType),
	new unifPar("emissionType","emissionType", valType),

	new unifPar("LAlightType","LAlightType", valType),
	new unifPar("LAPos","LAPos", valVec3),
	new unifPar("LADir","LADir", valDir),
	new unifPar("LAConeOut","LAConeOut", val),
	new unifPar("LAConeIn","LAConeIn", valD100),
	new unifPar("LADecay","LADecay", val),
	new unifPar("LATarget","LATarget", valD10),
	new unifPar("LAlightColor","LAlightColor", valCol),

	new unifPar("LBlightType","LBlightType", valType),
	new unifPar("LBPos","LBPos", valVec3),
	new unifPar("LBDir","LBDir", valDir),
	new unifPar("LBConeOut","LBConeOut", val),
	new unifPar("LBConeIn","LBConeIn", valD100),
	new unifPar("LBDecay","LBDecay", val),
	new unifPar("LBTarget","LBTarget", valD10),
	new unifPar("LBlightColor","LBlightColor", valCol),

	new unifPar("LClightType","LClightType", valType),
	new unifPar("LCPos","LCPos", valVec3),
	new unifPar("LCDir","LCDir", valDir),
	new unifPar("LCConeOut","LCConeOut", val),
	new unifPar("LCConeIn","LCConeIn", valD100),
	new unifPar("LCDecay","LCDecay", val),
	new unifPar("LCTarget","LCTarget", valD10),
	new unifPar("LClightColor","LClightColor", valCol),

	new unifPar("ambientLightColor","ambientLightColor", valCol),
	new unifPar("ambientLightLowColor","ambientLightLowColor", valCol),
	new unifPar("SHLeftLightColor","SHLeftLightColor", valCol),
	new unifPar("SHRightLightColor","SHRightLightColor", valCol),
	new unifPar("ADir","ADir", valDir),
	new unifPar("diffuseColor","diffuseColor", valCol),
	new unifPar("DTexMix","DTexMix", valD100),
	new unifPar("specularColor","specularColor", valCol),
	new unifPar("SpecShine","SpecShine", val),
	new unifPar("DToonTh","DToonTh", valD100),
	new unifPar("SToonTh","SToonTh", valD100),
	new unifPar("SspecKwAng","SspecKwAng", valD100),
	new unifPar("ambientMatColor","ambientMatColor", valCol),
	new unifPar("emitColor","emitColor", valCol),
	new unifPar("","u_texture", noAutoSet),
	new unifPar("","pMatrix", noAutoSet),
	new unifPar("","wMatrix", noAutoSet),
	new unifPar("","eyePos", noAutoSet)
];

function setXwingMesh() {
	// Load mesh using the webgl-obj-loader library
	worldMesh = new OBJ.Mesh(xwingObjStr);
	
	// Create the textures
	imgtx = new Image();
	imgtx.txNum = 0;
	imgtx.onload = textureLoaderCallback;
	imgtx.src = XwingTextureData;
	
	worldScale = 1.0;

	OBJ.initMeshBuffers(gl, worldMesh);
	
//	document.getElementById("diffuseColor").value = "#dddd88";
//	document.getElementById("ambientMatColor").value = "#dddd88";
	document.getElementById("my-canvas").style.backgroundColor = "#000033";
}

function setWorldMesh() {
	// Load mesh using the webgl-obj-loader library
	worldMesh = new OBJ.Mesh(worldObjStr);
	
	// Create the textures
	imgtx = new Image();
	imgtx.txNum = 0;
	imgtx.onload = textureLoaderCallback;
	imgtx.src = WorldTextureData;
	
	worldScale = 1.0;

	OBJ.initMeshBuffers(gl, worldMesh);
//	document.getElementById("diffuseColor").value = "#00ffff";
//	document.getElementById("ambientMatColor").value = "#00ffff";
	document.getElementById("my-canvas").style.backgroundColor = "#ffffff";
//	
//console.log(worldMesh.indexBuffer);
//console.log(worldMesh.vertexBuffer);
}

function setHouseMesh() {
	// Load mesh using the webgl-obj-loader library
	worldMesh = new OBJ.Mesh(houseObjStr);
	
	// Create the textures
	imgtx = new Image();
	imgtx.txNum = 0;
	imgtx.onload = textureLoaderCallback;
	imgtx.src = HouseTextureData;
	
	worldScale = 0.3;

	OBJ.initMeshBuffers(gl, worldMesh);
//	document.getElementById("diffuseColor").value = "#ffffff";
//	document.getElementById("ambientMatColor").value = "#ffffff";
	document.getElementById("my-canvas").style.backgroundColor = "#00aaaa";
}

function toggleExtView() {
	extView = 1-extView;
}

// texture loader callback
var textureLoaderCallback = function() {
	var textureId = gl.createTexture();
	gl.activeTexture(gl.TEXTURE0 + this.txNum);
	gl.bindTexture(gl.TEXTURE_2D, textureId);		
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this);		
// set the filtering so we don't need mips
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
}

// The real app starts here
function main(){
	resetShaderParams();
	
	// setup everything else
	var canvas = document.getElementById("my-canvas");
	canvas.addEventListener("mousedown", doMouseDown, false);
	canvas.addEventListener("mouseup", doMouseUp, false);
	canvas.addEventListener("mousemove", doMouseMove, false);
	canvas.addEventListener("mousewheel", doMouseWheel, false);
	window.addEventListener("keyup", keyFunctionUp, false);
	window.addEventListener("keydown", keyFunctionDown, false);
	window.onresize = doResize;
	canvas.width  = window.innerWidth-16;
	canvas.height = window.innerHeight-320;
	
	try{
		gl= canvas.getContext("webgl2");
	} catch(e){
		console.log(e);
	}
	
	if(gl){
		// Compile and link shaders
		program = gl.createProgram();
		var v1 = gl.createShader(gl.VERTEX_SHADER);
		gl.shaderSource(v1, vs);
		gl.compileShader(v1);
		if (!gl.getShaderParameter(v1, gl.COMPILE_STATUS)) {
			alert("ERROR IN VS SHADER : " + gl.getShaderInfoLog(v1));
		}
		var v2 = gl.createShader(gl.FRAGMENT_SHADER);
		gl.shaderSource(v2, fs)
		gl.compileShader(v2);		
		if (!gl.getShaderParameter(v2, gl.COMPILE_STATUS)) {
			alert("ERROR IN FS SHADER : " + gl.getShaderInfoLog(v2));
		}			
		gl.attachShader(program, v1);
		gl.attachShader(program, v2);
		gl.linkProgram(program);				
		
		gl.useProgram(program);

		setWorldMesh();
		
		// links mesh attributes to shader attributes
		program.vertexPositionAttribute = gl.getAttribLocation(program, "in_pos");
		gl.enableVertexAttribArray(program.vertexPositionAttribute);
		 
		program.vertexNormalAttribute = gl.getAttribLocation(program, "in_norm");
		gl.enableVertexAttribArray(program.vertexNormalAttribute);
		 
		program.textureCoordAttribute = gl.getAttribLocation(program, "in_uv");
		gl.enableVertexAttribArray(program.textureCoordAttribute);

		for(var i = 0; i < unifParArray.length; i++) {
			program[unifParArray[i].pGLSL+"Uniform"] = gl.getUniformLocation(program, unifParArray[i].pGLSL);
		}
				
		// prepares the world, view and projection matrices.
		canw=canvas.clientWidth;
		canh=canvas.clientHeight;
		
		gl.clearColor(0.0, 0.0, 0.0, 1.0);
//		gl.viewport(0.0, 0.0, w, h);
		gl.clear(gl.COLOR_BUFFER_BIT);
				
	 // turn on depth testing
	    gl.enable(gl.DEPTH_TEST);

		NvertexBuffer = gl.createBuffer();
		NnormalBuffer = gl.createBuffer();
		NtextureBuffer = gl.createBuffer();
		NindexBuffer = gl.createBuffer();
		
		drawScene();
	}else{
		alert("Error: WebGL not supported by your browser!");
	}
}

function drawCone(s, h, r, mat, cr, cg, cb) {
	var Nvertices = [0, 0, 0];
	var Nnormals = [0, 0, 1];
	var Ntextures = [0.5, 0.5];
	var Nindices = [];
	
	for(i=0;i<s;i++) {
		Nnormals[3*i+3] = 0; Nnormals[3*i+4] = 0; Nnormals[3*i+5] = 1;
		Ntextures[2*i+2] = 0.5; Ntextures[2*i+3] = 0.5;
		Nvertices[3*i+3] = r*Math.cos(2*(i+0.5)/s*Math.PI);
		Nvertices[3*i+4] = r*Math.sin(2*(i+0.5)/s*Math.PI);
		Nvertices[3*i+5] = -h;
		
		Nindices[4*i] = 0;
		Nindices[4*i+1] = i+1;
		Nindices[4*i+2] = i+1;
		Nindices[4*i+3] = (i+1) % s + 1;
	}
		
	Nvertices = new Float32Array(Nvertices);
	gl.bindBuffer(gl.ARRAY_BUFFER, NvertexBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, Nvertices.byteLength, gl.STATIC_DRAW);
	gl.bufferSubData(gl.ARRAY_BUFFER, 0, Nvertices);
	gl.vertexAttribPointer(program.vertexPositionAttribute, 3, gl.FLOAT, false, 0, 0);

	Nnormals = new Float32Array(Nnormals);
	gl.bindBuffer(gl.ARRAY_BUFFER, NnormalBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, Nnormals.byteLength, gl.STATIC_DRAW);
	gl.bufferSubData(gl.ARRAY_BUFFER, 0, Nnormals);
	gl.vertexAttribPointer(program.vertexNormalAttribute, 3, gl.FLOAT, false, 0, 0);

	Ntextures = new Float32Array(Ntextures);
	gl.bindBuffer(gl.ARRAY_BUFFER, NtextureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, Ntextures.byteLength, gl.STATIC_DRAW);
	gl.bufferSubData(gl.ARRAY_BUFFER, 0, Ntextures);
	gl.vertexAttribPointer(program.textureCoordAttribute, 2, gl.FLOAT, false, 0, 0);

	Nindices = new Uint16Array(Nindices);
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, NindexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, Nindices.byteLength, gl.STATIC_DRAW);
	gl.bufferSubData(gl.ELEMENT_ARRAY_BUFFER, 0, Nindices);	

	gl.uniformMatrix4fv(program.pMatrixUniform, gl.FALSE, utils.transposeMatrix(mat));
	
	gl.uniform4f(program["ambientTypeUniform"], 0, 0, 0, 0);
	gl.uniform4f(program["diffuseTypeUniform"], 0, 0, 0, 0);
	gl.uniform4f(program["specularTypeUniform"], 0, 0, 0, 0);
	gl.uniform4f(program["emissionTypeUniform"], 1, 0, 0, 0);
	gl.uniform4f(program["emitColorUniform"], cr, cg, cb, 0);
	gl.uniform1f(program["DTexMixUniform"], 0);
	gl.drawElements(gl.LINES, 4*s, gl.UNSIGNED_SHORT, 0);
}

function drawLight(L, mat, cr, cg, cb) {

	var ConeOut = document.getElementById(L+"ConeOut").value / 360 * 3.14;
	var ConeIn = document.getElementById(L+"ConeIn").value / 100;

	var px = document.getElementById(L+"PosX").value / 10;
	var py = document.getElementById(L+"PosY").value / 10;
	var pz = document.getElementById(L+"PosZ").value / 10;

	var t = utils.degToRad(document.getElementById(L+"DirTheta").value) / 3.14 * 180 - 90;
	var p = utils.degToRad(document.getElementById(L+"DirPhi").value) / 3.14 * 180;
//	var dx = Math.sin(t)*Math.sin(p);
//	var dy = Math.cos(t);
//	var dz = Math.sin(t)*Math.cos(p);
	
	var type = valTypeDecoder[L+"lightType"][document.getElementById(L+"lightType").value];

//console.log(type + " " + px + " " + py + " " + pz + " "
//				 	   + dx + " " + dy + " " + dz + " " + ConeOut + " " + ConeIn);
	if(type[0] > 0) {
		// point light
		var WVP = utils.multiplyMatrices(
					mat,
					utils.MakeCamera(0, 0, 0, t+180, p)
		);		
		drawCone(4,20,0.1, WVP, cr, cg, cb);
	}
	if(type[1] > 0) {
		// direct light
		var WVP = utils.multiplyMatrices(
					mat,
					utils.MakeCamera(px, py, pz+1, 0, 0)
		);		
		drawCone(4,1,1, WVP, cr, cg, cb);
		var WVP = utils.multiplyMatrices(
					mat,
					utils.MakeCamera(px, py, pz-1, 0, 180)
		);		
		drawCone(4,1,1, WVP, cr, cg, cb);
	}
	if(type[2] > 0) {
		// spot light
		var L = 25;
		var WVP = utils.multiplyMatrices(
					mat,
					utils.MakeCamera(px, py, pz, t, p)
		);		
		drawCone(12, L, L*Math.tan(ConeOut*ConeIn), WVP, cr*0.7, cg*0.7, cb*0.7);
		drawCone(12, L, L*Math.tan(ConeOut), WVP, cr, cg, cb);
	}
}


function drawAmbient(mat) {

	
	var type = valTypeDecoder["ambientType"][document.getElementById("ambientType").value];	
	var c0 = document.getElementById("ambientLightColor").value;
	var c1 = document.getElementById("ambientLightLowColor").value;
	var c2 = document.getElementById("SHLeftLightColor").value;
	var c3 = document.getElementById("SHRightLightColor").value;
	var ath = document.getElementById("ADirTheta").value;
	var aph = document.getElementById("ADirPhi").value;


//console.log(type + " " + c0 + " " + c1 + " " + c2 + " " + c3 + " " + ath + " " + aph);

	if(type[0] > 0) {
		// ambient
		var WVP = utils.multiplyMatrices(
					mat,
					utils.MakeCamera(0, -10, 0, 90, 0)
		);
		col = c0;
	    R = parseInt(col.substring(1,3) ,16) / 255;
	    G = parseInt(col.substring(3,5) ,16) / 255;
	    B = parseInt(col.substring(5,7) ,16) / 255;
		drawCone(64,0,10, WVP, R,G,B);
	}
	if(type[1] > 0) {
		// emispheric
		var WVP = utils.multiplyMatrices(
					mat,utils.multiplyMatrices(utils.MakeCamera(0, 0, 0, ath, aph),
					utils.multiplyMatrices(utils.MakeRotateXMatrix(90), utils.MakeTranslateMatrix(0,0,-10)))
		);
		col = c0;
	    R = parseInt(col.substring(1,3) ,16) / 255;
	    G = parseInt(col.substring(3,5) ,16) / 255;
	    B = parseInt(col.substring(5,7) ,16) / 255;
		drawCone(32,0,4, WVP, R,G,B);
		
		
		var WVP = utils.multiplyMatrices(
					mat,utils.multiplyMatrices(utils.MakeCamera(0, 0, 0, ath, aph),
					utils.multiplyMatrices(utils.MakeRotateXMatrix(90), utils.MakeTranslateMatrix(0,0,10)))
		);
		col = c1;
	    R = parseInt(col.substring(1,3) ,16) / 255;
	    G = parseInt(col.substring(3,5) ,16) / 255;
	    B = parseInt(col.substring(5,7) ,16) / 255;
		drawCone(42,0,4, WVP, R,G,B);
	}
	if(type[2] > 0) {
		// emispheric
		var WVP = utils.multiplyMatrices(
					mat,utils.multiplyMatrices(utils.MakeTranslateMatrix(0,10,0), utils.MakeRotateXMatrix(90))
		);
		col = c0;
	    R = parseInt(col.substring(1,3) ,16) / 255;
	    G = parseInt(col.substring(3,5) ,16) / 255;
	    B = parseInt(col.substring(5,7) ,16) / 255;
		drawCone(32,0,3, WVP, R,G,B);
		
		var WVP = utils.multiplyMatrices(
					mat,
					utils.multiplyMatrices(utils.MakeRotateXMatrix(120), utils.multiplyMatrices(utils.MakeTranslateMatrix(0,10,0), utils.MakeRotateXMatrix(90)))
		);
		col = c1;
	    R = parseInt(col.substring(1,3) ,16) / 255;
	    G = parseInt(col.substring(3,5) ,16) / 255;
	    B = parseInt(col.substring(5,7) ,16) / 255;
		drawCone(42,0,3, WVP, R,G,B);
		
		var WVP = utils.multiplyMatrices(
					mat,utils.multiplyMatrices(utils.MakeRotateYMatrix(-120),
					utils.multiplyMatrices(utils.MakeRotateXMatrix(120), utils.multiplyMatrices(utils.MakeTranslateMatrix(0,10,0), utils.MakeRotateXMatrix(90))))
		);
		col = c2;
	    R = parseInt(col.substring(1,3) ,16) / 255;
	    G = parseInt(col.substring(3,5) ,16) / 255;
	    B = parseInt(col.substring(5,7) ,16) / 255;
		drawCone(42,0,3, WVP, R,G,B);

		var WVP = utils.multiplyMatrices(
					mat,utils.multiplyMatrices(utils.MakeRotateYMatrix(120),
					utils.multiplyMatrices(utils.MakeRotateXMatrix(120), utils.multiplyMatrices(utils.MakeTranslateMatrix(0,10,0), utils.MakeRotateXMatrix(90))))
		);
		col = c3;
	    R = parseInt(col.substring(1,3) ,16) / 255;
	    G = parseInt(col.substring(3,5) ,16) / 255;
	    B = parseInt(col.substring(5,7) ,16) / 255;
		drawCone(42,0,3, WVP, R,G,B);
	}
}


function drawScene() {
//		gl.clearColor(0.0, 0.0, 0.0, 1.0);
//		gl.clear(gl.COLOR_BUFFER_BIT);

	if(extView > 0) {
		gl.viewport(0.0, 0.0, canw*0.66, canh);
		perspectiveMatrix = utils.MakePerspective(60, canw*0.66/canh, 0.1, 1000.0);
	} else {
		gl.viewport(0.0, 0.0, canw, canh);
		perspectiveMatrix = utils.MakePerspective(60, canw/canh, 0.1, 1000.0);
	}


	// update WV matrix
	
	angle = angle + rvy;
	elevation = elevation + rvx;
	
	cz = lookRadius * Math.cos(utils.degToRad(-angle)) * Math.cos(utils.degToRad(-elevation));
	cx = lookRadius * Math.sin(utils.degToRad(-angle)) * Math.cos(utils.degToRad(-elevation));
	cy = lookRadius * Math.sin(utils.degToRad(-elevation));
	viewMatrix = utils.MakeView(cx, cy, cz, elevation, -angle);

		// Magic for moving the car
		modRot = document.getElementById("modRot").value;
		worldMatrix = utils.MakeWorld(0,0,0, modRot,0,0, worldScale);

		projectionMatrix = utils.multiplyMatrices(perspectiveMatrix, viewMatrix);		

		
		// draws the request
		gl.bindBuffer(gl.ARRAY_BUFFER, worldMesh.vertexBuffer);
		gl.vertexAttribPointer(program.vertexPositionAttribute, worldMesh.vertexBuffer.itemSize, gl.FLOAT, false, 0, 0);
	    gl.bindBuffer(gl.ARRAY_BUFFER, worldMesh.textureBuffer);
	    gl.vertexAttribPointer(program.textureCoordAttribute, worldMesh.textureBuffer.itemSize, gl.FLOAT, false, 0, 0);
		
		gl.bindBuffer(gl.ARRAY_BUFFER, worldMesh.normalBuffer);
		gl.vertexAttribPointer(program.vertexNormalAttribute, worldMesh.normalBuffer.itemSize, gl.FLOAT, false, 0, 0);
		 
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, worldMesh.indexBuffer);		

		gl.uniform1i(program.u_textureUniform, 0);
		gl.uniform3f(program.eyePosUniform, cx, cy, cz);
		WVPmatrix = utils.multiplyMatrices(projectionMatrix, worldMatrix);
		gl.uniformMatrix4fv(program.pMatrixUniform, gl.FALSE, utils.transposeMatrix(WVPmatrix));
		gl.uniformMatrix4fv(program.wMatrixUniform, gl.FALSE, utils.transposeMatrix(worldMatrix));
		
		for(var i = 0; i < unifParArray.length; i++) {
			unifParArray[i].type(gl);
		}

		gl.drawElements(gl.TRIANGLES, worldMesh.indexBuffer.numItems, gl.UNSIGNED_SHORT, 0);


		if(extView == 1) {
			gl.viewport(canw*0.66+1.0, 0.0, canw*0.33, canh);
	
			EVcz = 80 * Math.cos(utils.degToRad(-EVangle)) * Math.cos(utils.degToRad(-EVelevation));
			EVcx = 80 * Math.sin(utils.degToRad(-EVangle)) * Math.cos(utils.degToRad(-EVelevation));
			EVcy = 80 * Math.sin(utils.degToRad(-EVelevation));
			viewMatrix = utils.MakeView(EVcx, EVcy, EVcz, EVelevation, -EVangle);

			parPrj = utils.MakeParallel(20, canw*0.33/canh, 0.1, 1000.0);
			projectionMatrix = utils.multiplyMatrices(
						parPrj,
						viewMatrix
			);		
			WVPmatrix = utils.multiplyMatrices(projectionMatrix, worldMatrix);
			gl.uniformMatrix4fv(program.pMatrixUniform, gl.FALSE, utils.transposeMatrix(WVPmatrix));
			gl.drawElements(gl.TRIANGLES, worldMesh.indexBuffer.numItems, gl.UNSIGNED_SHORT, 0);
	
	
			WVP = utils.multiplyMatrices(
						projectionMatrix,
						utils.MakeCamera(cx, cy, cz, elevation, -angle)
			);		
			drawCone(4,10,8, WVP, 0.75, 0.75, 0.75);
			
			drawLight("LA", projectionMatrix, 1, 0, 0);
			drawLight("LB", projectionMatrix, 1, 0.5, 0);
			drawLight("LC", projectionMatrix, 1, 1, 0);
			
			drawAmbient(projectionMatrix);
		}
				
		window.requestAnimationFrame(drawScene);		
}