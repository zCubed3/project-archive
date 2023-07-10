using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace zCubed.MSDF
{
    public static class Texture2DExtensions
    {
        public static Vector2 GetSize(this Texture2D tex) => new Vector2(tex.Width, tex.Height);
        public static Vector2 NormalizeToSize(this Texture2D tex, Vector2 a) => Vector2.Divide(a, tex.GetSize());
        public static Vector2 GetTexelSize(this Texture2D tex) => Vector2.Divide(Vector2.One, tex.GetSize());
    }
}
