using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace Opal
{
    public static class OpalEditorHelpers
    {
        public static bool AskTexture(ref Material material, string property, string name)
        {
            GUILayout.BeginHorizontal();

            GUILayout.Label(name, EditorStyles.boldLabel);
            material.SetTexture(property, EditorGUILayout.ObjectField(material.GetTexture(property), typeof(Texture2D), false) as Texture);

            GUILayout.EndHorizontal();

            return material.GetTexture(property) != null;
        }

        public static bool AskBool(ref Material material, string property, string name)
        {
            GUILayout.BeginHorizontal();

            GUILayout.Label(name, EditorStyles.boldLabel);
            material.SetInt(property, EditorGUILayout.Toggle(material.GetInt(property) != 0) ? 1 : 0);

            GUILayout.EndHorizontal();

            return material.GetInt(property) != 0;
        }

        public static float AskFloat(ref Material material, string property, string name)
        {
            GUILayout.BeginHorizontal();

            GUILayout.Label(name, EditorStyles.boldLabel);
            material.SetFloat(property, EditorGUILayout.FloatField(material.GetFloat(property)));

            GUILayout.EndHorizontal();

            return material.GetFloat(property);
        }

        public static float AskFloatRange(ref Material material, string property, string name, float min = 0F, float max = 1F)
        {
            GUILayout.BeginHorizontal();

            GUILayout.Label(name, EditorStyles.boldLabel);
            material.SetFloat(property, EditorGUILayout.Slider(material.GetFloat(property), min, max));

            GUILayout.EndHorizontal();

            return material.GetFloat(property);
        }

        public static Color AskColor(ref Material material, string property, string name)
        {
            GUILayout.BeginHorizontal();

            GUILayout.Label(name, EditorStyles.boldLabel);
            material.SetColor(property, EditorGUILayout.ColorField(material.GetColor(property)));

            GUILayout.EndHorizontal();

            return material.GetColor(property);
        }

        public static Color AskColorHDR(ref Material material, string property, string name)
        {
            GUILayout.BeginHorizontal();

            GUILayout.Label(name, EditorStyles.boldLabel);
            material.SetColor(property, EditorGUILayout.ColorField(GUIContent.none, material.GetColor(property), true, false, true));

            GUILayout.EndHorizontal();

            return material.GetColor(property);
        }

        public static void AskScaleOffsetInfo(ref Material material, string property, string name)
        {
            GUILayout.Label(name, EditorStyles.boldLabel);

            Vector2 offset = material.GetTextureOffset(property);
            Vector2 scale = material.GetTextureScale(property);

            GUILayout.BeginHorizontal();

            offset.x = EditorGUILayout.FloatField(offset.x);
            offset.y = EditorGUILayout.FloatField(offset.y);
            material.SetTextureOffset(property, offset);

            GUILayout.EndHorizontal();

            GUILayout.BeginHorizontal();

            scale.x = EditorGUILayout.FloatField(scale.x);
            scale.y = EditorGUILayout.FloatField(scale.y);
            material.SetTextureScale(property, scale);

            GUILayout.EndHorizontal();
        }

        public static T AskEnum<T>(ref Material material, string property, string name) where T : System.Enum
        {
            //T enumValue = System.Enum.Parse<T>(material.GetInteger(property).ToString(), true);
            T enumValue = (T)(object)material.GetInteger(property);
            enumValue = (T)EditorGUILayout.EnumPopup(name, enumValue);
            material.SetInteger(property, (int)(object)enumValue);

            return enumValue;
        }
    }
}