using System.Collections;
using System.Collections.Generic;

using UnityEngine;
using UnityEngine.Rendering;

using UnityEditor;

namespace Opal
{
    public class OpalToonEditor : OpalShaderEditor
    {
        protected enum DrawingMode 
        {
            Opaque,
            Transparent
        }

        protected override string ShaderName => "Toon";

        protected DrawingMode mode;

        protected override void DoMaterial(ref Material material)
        {
            GUILayout.Label("Shading Model", CenterBoldLargeStyle);
            GUILayout.Space(10);

            switch (OpalEditorHelpers.AskEnum<DrawingMode>(ref material, "_DrawingMode", "Drawing Mode"))
            {
                case DrawingMode.Opaque:
                    material.SetFloat("_BlendSrc", (float)BlendMode.One);
                    material.SetFloat("_BlendDst", (float)BlendMode.Zero);
                    material.renderQueue = (int)RenderQueue.Geometry;
                    break;

                case DrawingMode.Transparent:
                    material.SetFloat("_BlendSrc", (float)BlendMode.SrcAlpha);
                    material.SetFloat("_BlendDst", (float)BlendMode.OneMinusSrcAlpha);
                    material.renderQueue = (int)RenderQueue.Transparent;
                    break;
            }

            GUILayout.Space(10);
            GUILayout.Label("Surface", CenterBoldLargeStyle);
            GUILayout.Space(10);

            OpalEditorHelpers.AskColor(ref material, "_Color", "Color");
            OpalEditorHelpers.AskTexture(ref material, "_MainTex", "Texture");

            GUILayout.Space(5);

            OpalEditorHelpers.AskFloatRange(ref material, "_Specular", "Specular");
            OpalEditorHelpers.AskFloatRange(ref material, "_Roughness", "Roughness");
            OpalEditorHelpers.AskFloatRange(ref material, "_Metallic", "Metallic");

            GUILayout.Space(10);
            GUILayout.Label("Outline", CenterBoldLargeStyle);
            GUILayout.Space(10);

            //bool outline = OpalEditorHelpers.AskBool(ref material, "_OutlineEnabled", "Outline Enabled?");
            bool outline = true;

            // Unity fix your shit, why the hell does this only apply to lightmode tags?
            //material.SetShaderPassEnabled("Outline", outline);

            // +1 Unity fix your shit, disabling the ztesting on the outline somehow breaks the shadows?
            /*
            if (outline)
            {
                material.SetFloat("_OutlineZTest", (float)CompareFunction.Less);
                material.EnableKeyword("_OUTLINE_ENABLED");
            }
            else
            {
                material.SetFloat("_OutlineZTest", (float)CompareFunction.Never);
                material.DisableKeyword("_OUTLINE_ENABLED");
            }
            */

            using (new EditorGUI.DisabledGroupScope(!outline))
            {
                OpalEditorHelpers.AskFloatRange(ref material, "_OutlineExtrusion", "Extrusion");
                OpalEditorHelpers.AskColorHDR(ref material, "_OutlineColor", "Color");
            }

            bool stenciling = OpalEditorHelpers.AskBool(ref material, "_OutlineStencilEnabled", "Enable Stenciling");

            using (new EditorGUI.DisabledGroupScope(!stenciling))
                OpalEditorHelpers.AskFloatRange(ref material, "_OutlineStencilRef", "Stencil Index", 0, 255);

            material.SetFloat("_OutlineOperation", (int)(stenciling ? CompareFunction.NotEqual : CompareFunction.Always));

            GUILayout.Space(10);
            GUILayout.Label("Lighting", CenterBoldLargeStyle);
            GUILayout.Space(10);

            OpalEditorHelpers.AskFloatRange(ref material, "_ShadingBands", "Light Bands", 1, 16);
        }
    }
}