using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace Opal
{
    public abstract class OpalShaderEditor : ShaderGUI
    {
        protected static GUIStyle OpalLogoStyle;
        protected static GUIStyle CenterBoldStyle;
        protected static GUIStyle CenterBoldLargeStyle;

        protected abstract string ShaderName { get; }
        protected virtual bool DisplaySDKWarning => true;

        protected virtual void InitializeStyles()
        {
            if (OpalLogoStyle == null)
            {
                OpalLogoStyle = new GUIStyle(EditorStyles.boldLabel);
                OpalLogoStyle.alignment = TextAnchor.MiddleCenter;
                OpalLogoStyle.fontStyle = FontStyle.Bold;
                OpalLogoStyle.fontSize = 24;
            }

            if (CenterBoldStyle == null)
            {
                CenterBoldStyle = new GUIStyle(EditorStyles.boldLabel);
                CenterBoldStyle.alignment = TextAnchor.MiddleCenter;
            }

            if (CenterBoldLargeStyle == null)
            {
                CenterBoldLargeStyle = new GUIStyle(EditorStyles.boldLabel);
                CenterBoldLargeStyle.alignment = TextAnchor.MiddleCenter;
                CenterBoldLargeStyle.fontSize += 4;
            }
        }

        protected virtual void DoTitle()
        {
            GUILayout.Label($"Opal - {ShaderName}", OpalLogoStyle);

            if (DisplaySDKWarning)
                GUILayout.Label("Ignore any SDK keyword errors, variants are required!", CenterBoldStyle);

            GUILayout.Space(10);
        }

        protected abstract void DoMaterial(ref Material material);

        public override void OnGUI(MaterialEditor materialEditor, MaterialProperty[] properties)
        {
            InitializeStyles();
            DoTitle();

            Material material = materialEditor.target as Material;
            Undo.RecordObject(material, "Changed parameters");

            DoMaterial(ref material);

            //base.OnGUI(materialEditor, properties);
        }
    }
}