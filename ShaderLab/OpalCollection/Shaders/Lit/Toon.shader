Shader "Opal/Lit/Toon"
{
    Properties
    {
        [Header(Material)]
        _MainTex ("Texture", 2D) = "white" {}
        _Color ("Color", Color) = (1.0, 1.0, 1.0, 1.0)

        _Specular ("Specular", Range(0, 1)) = 0.0
        _Roughness ("Roughness", Range(0, 1)) = 1.0
        _Metallic ("Metallic", Range(0, 1)) = 0.0

        [Header(Lighting)]
        _ShadingBands ("Bands", Int) = 2
        //_ShadingPower ("Softness", Range(0.01, 1)) = 1.0
        //_AmbientBoost ("Ambient Boost", Range(0, 1)) = 0.1

        [Header(Outline)]
        [HDR] _OutlineColor ("Color", Color) = (0.0, 0.0, 0.0, 1.0)
        _OutlineExtrusion ("Extrusion", float) = 0.001

        [Header(Meta Info)]
        _DrawingMode ("Drawing Mode", Integer) = 0
        _BlendSrc ("Blend Src", float) = 1
        _BlendDst ("Blend Dst", float) = 0
        //_OutlineEnabled ("Outline Enabled", float) = 1
        //_OutlineZTest ("Outline ZTest", float) = 2
        //_CastShadows ("Cast Shadows", float) = 1
        //_ShadowZTest ("Shadow ZTest", float) = 2
        _OutlineStencilEnabled ("Stencil Enabled", float) = 0
        _OutlineStencilRef ("Stencil Ref", float) = 2
        _OutlineOperation ("Stencil Op", float) = 6
    }

    CustomEditor "Opal.OpalToonEditor"

    SubShader
    {
        LOD 100
        
        //
        // Common
        //
        HLSLINCLUDE
            sampler2D _MainTex;
            half4 _Color;
            int _ShadingBands;
            //float _ShadingPower;
            //float _AmbientBoost;
            float _Specular;
            float _Roughness;
            float _Metallic;
        ENDHLSL

        //
        // Forward Passes
        // 
        Pass
        {
            Name "Forward Base"

            Tags { 
                "RenderType"="Opaque" 
                "LightMode"="ForwardBase" 
            }

            Stencil {
                Ref [_OutlineStencilRef]
                Comp Always
                Pass Replace
            }
            
            Blend [_BlendSrc] [_BlendDst]

            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #pragma multi_compile_fog
            #pragma multi_compile_fwdbase
            
            #pragma target 5.0

            #define TOON_FORWARD_BASE
            #include "ToonCommon.hlsl"

            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;

                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct v2f
            {
                float4 pos : SV_POSITION;
                float3 positionWS : TEXCOORD0;
                float3 normalWS : TEXCOORD1;
                float2 uv : TEXCOORD2;
                float3 ambient : TEXCOORD3;

                SHADOW_COORDS(4)
                UNITY_FOG_COORDS(5)

                UNITY_VERTEX_OUTPUT_STEREO
            };

            v2f vert (appdata v)
            {
                v2f o;

                UNITY_SETUP_INSTANCE_ID(v)
                UNITY_INITIALIZE_OUTPUT(v2f, o);
                UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);

                o.pos = UnityObjectToClipPos(v.vertex);
                o.positionWS = mul(unity_ObjectToWorld, v.vertex);
                o.normalWS = UnityObjectToWorldNormal(v.normal);
                o.uv = v.uv;
                
                o.ambient = half3(unity_SHAr.w, unity_SHAg.w, unity_SHAb.w);
                //o.ambient = ShadeSH9(float4(o.normal, 1));

                TRANSFER_SHADOW(o);
                UNITY_TRANSFER_FOG(o, o.pos);

                return o;
            }

            half4 frag (v2f i) : SV_Target
            {
                UNITY_SETUP_STEREO_EYE_INDEX_POST_VERTEX(i);

                ToonInput toonInput = (ToonInput)0;

                toonInput.positionWS = i.positionWS;
                toonInput.viewWS = normalize(_WorldSpaceCameraPos - i.positionWS);
                toonInput.normalWS = normalize(i.normalWS);
                TOON_TRANSFER_SHADOW(i, toonInput);

                ToonMaterial toonMat = (ToonMaterial)0;
                toonMat.albedo = tex2D(_MainTex, i.uv) * _Color;
                toonMat.roughness = _Roughness;
                toonMat.metallic = _Metallic;
                toonMat.specular = _Specular;
                toonMat.ambient = i.ambient * toonMat.albedo;
                toonMat.bands = _ShadingBands;

                return ShadeToon(toonInput, toonMat);
            }
            ENDHLSL
        }
        Pass
        {
            Tags { 
                "RenderType"="Opaque" 
                "LightMode"="ForwardAdd" 
            }

            Blend One One 
            ZWrite Off
            //ZTest Equal

            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #pragma multi_compile_fog
            #pragma multi_compile_fwdadd_fullshadows
            
            #pragma target 5.0

            #include "ToonCommon.hlsl"

            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;

                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct v2f
            {
                float4 pos : SV_POSITION;
                float3 positionWS : TEXCOORD0;
                float3 normalWS : TEXCOORD1;
                float2 uv : TEXCOORD2;

                SHADOW_COORDS(4)
                UNITY_FOG_COORDS(5)

                UNITY_VERTEX_OUTPUT_STEREO
            };

            v2f vert (appdata v)
            {
                v2f o;

                UNITY_SETUP_INSTANCE_ID(v)
                UNITY_INITIALIZE_OUTPUT(v2f, o);
                UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);

                o.pos = UnityObjectToClipPos(v.vertex);
                o.positionWS = mul(unity_ObjectToWorld, v.vertex);
                o.normalWS = UnityObjectToWorldNormal(v.normal);
                o.uv = v.uv;

                TRANSFER_SHADOW(o);
                UNITY_TRANSFER_FOG(o, o.pos);

                return o;
            }

            half4 frag (v2f i) : SV_Target
            {
                UNITY_SETUP_STEREO_EYE_INDEX_POST_VERTEX(i);

                ToonInput toonInput = (ToonInput)0;

                toonInput.positionWS = i.positionWS;
                toonInput.viewWS = normalize(_WorldSpaceCameraPos - i.positionWS);
                toonInput.normalWS = normalize(i.normalWS);
                TOON_TRANSFER_SHADOW(i, toonInput);

                ToonMaterial toonMat = (ToonMaterial)0;
                toonMat.albedo = tex2D(_MainTex, i.uv) * _Color;
                toonMat.roughness = _Roughness;
                toonMat.metallic = _Metallic;
                toonMat.specular = _Specular;
                toonMat.bands = _ShadingBands;

                return ShadeToon(toonInput, toonMat);
            }
            ENDHLSL
        }
        Pass 
        {
            Name "Outline"

            Tags { 
                "RenderType"="Opaque" 
                "LightMode"="Always" 
            }

            Stencil {
                Ref [_OutlineStencilRef]
                Comp [_OutlineOperation]
            }

            Cull Front
            //ZTest [_OutlineZTest]
            ZWrite Off

            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #pragma multi_compile_fog
            #pragma multi_compile _ _OUTLINE_ENABLED

            #pragma target 5.0

            #include "UnityCG.cginc"

            float _OutlineExtrusion;
            half4 _OutlineColor;

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
                float3 normal : NORMAL;

                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct v2f
            {
                float4 pos : SV_POSITION;

                UNITY_VERTEX_OUTPUT_STEREO
            };

            v2f vert (appdata v)
            {
                v2f o;

                UNITY_SETUP_INSTANCE_ID(v)
                UNITY_INITIALIZE_OUTPUT(v2f, o);
                UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);

                float3 offset = normalize(v.normal) * _OutlineExtrusion;
                o.pos = UnityObjectToClipPos(v.vertex + offset);

                return o;
            }

            half4 frag (v2f i) : SV_Target
            {
                UNITY_SETUP_STEREO_EYE_INDEX_POST_VERTEX(i);

                half4 color = _OutlineColor;
                UNITY_APPLY_FOG(i.fogCoord, color);
                
                return color;
            }


            ENDHLSL
        }
        Pass
        {
            Tags { 
                "LightMode"="ShadowCaster" 
            }

            //ZWrite [_CastShadows]
            //ZTest [_ShadowZTest]

            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #pragma multi_compile_shadowcaster

            #include "UnityCG.cginc"

            struct appdata {
                float4 vertex : POSITION;
            };

            struct v2f { 
                float4 positionCS : SV_POSITION;
            };

            v2f vert(appdata v)
            {
                v2f o;
                
                o.positionCS = UnityObjectToClipPos(v.vertex);

                return o;
            }

            float4 frag(v2f i) : SV_Target
            {
                return 0;
            }
            ENDCG
        }
    }
}
