Texture2D ShaderTexture;

SamplerState Sampler {

	AddressU = Wrap;
	AddressV = Wrap;
	Filter = MIN_MAG_MIP_LINEAR;
};



float4 main(float4 position : SV_POSITION, float2 tex : TEXCOORD0) : SV_TARGET
{
	float4 result = ShaderTexture.Sample(Sampler, tex);

	/* grayscale
	result[0] = result[1] = result[2] = result[3] = result[0] * 0.21f + result[1] * 0.72f + result[2] * 0.07f;
	
	// 3°³ »ö±òÀÇ Æò±Õ ¹æ¹ıµµ ÀÖÀ½ 
	//result[0] = result[1] = result[2] = (result[0] + result[1] + result[2]) / 3;

	
	if (result[0] > 0.6f)
	{
		result[0] = result[1] = result[2] = 1.0f;
	}
	else{
		result[0] = result[1] = result[2] = 0.0f
	}
	*/

	return result;
}