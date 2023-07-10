using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

using Newtonsoft.Json;

namespace zCubed.MSDF
{
    /// <summary>
    /// Alternative to MonoGame's built in "SpriteFont"
    /// </summary>
    public class ScalableFont
    {
        public class GlyphQuad
        {
            public readonly VertexPositionTexture[] vertices;
            public readonly short[] indices;

            public GlyphQuad()
            {
                vertices = new[]
                {
                    new VertexPositionTexture(
                        new Vector3(0, 0, 0),
                        new Vector2(1, 1)),
                    new VertexPositionTexture(
                        new Vector3(0, 0, 0),
                        new Vector2(0, 1)),
                    new VertexPositionTexture(
                        new Vector3(0, 0, 0),
                        new Vector2(0, 0)),
                    new VertexPositionTexture(
                        new Vector3(0, 0, 0),
                        new Vector2(1, 0))
                };

                indices = new short[] { 0, 1, 2, 2, 3, 0 };
            }


            public void Render(GraphicsDevice device, Vector2 v1, Vector2 v2)
            {
                vertices[0].Position.X = v2.X;
                vertices[0].Position.Y = v1.Y;

                vertices[1].Position.X = v1.X;
                vertices[1].Position.Y = v1.Y;

                vertices[2].Position.X = v1.X;
                vertices[2].Position.Y = v2.Y;

                vertices[3].Position.X = v2.X;
                vertices[3].Position.Y = v2.Y;

                device.DrawUserIndexedPrimitives(
                    PrimitiveType.TriangleList,
                    vertices,
                    0,
                    4,
                    indices,
                    0,
                    2);
            }
        }

        public static Game ActiveGame = null;
        public static Dictionary<string, ScalableFont> LoadedFonts = new Dictionary<string, ScalableFont>();
        public static Effect MSDFShader = null;
        public static DrawingLayer FontDrawingLayer = new DrawingLayer();
        public static GlyphQuad Quad = new GlyphQuad();
        public static RasterizerState RasterizerState = new RasterizerState() { CullMode = CullMode.None };

        public class MSDFAtlas
        {
            public string type;
            public float distanceRange;
            public float size;
            public int width;
            public int height;
            public string yOrigin;
        }

        public class MSDFMetrics
        {
            public float emSize;
            public float lineHeight;
            public float ascender;
            public float descender;
            public float underlineY;
            public float underlineThickness;
        }

        public class MSDFGlyphBounds
        {
            public float left;
            public float bottom;
            public float right;
            public float top;
        }

        public class MSDFGlyph
        {
            public char unicode;
            public float advance;
            public MSDFGlyphBounds planeBounds;
            public MSDFGlyphBounds atlasBounds;

            public float width;
            public float height;

            public void CalculateLayout(MSDFAtlas atlas)
            {
                width = (atlasBounds.right - atlasBounds.left) / atlas.size;
                height = (atlasBounds.bottom - atlasBounds.top) / atlas.size;
            }
        }


        public MSDFAtlas atlas;
        public MSDFMetrics metrics;
        public MSDFGlyph[] glyphs;
        public float whiteSpaceWidth = 0.3F; // Multiplier over size of 1 character


        public Dictionary<char, MSDFGlyph> GlyphTable { get; private set; } = new Dictionary<char, MSDFGlyph>();
        public string FontName { get; private set; }

        [NonSerialized]
        public Texture2D atlasTexture;


        public static ScalableFont Load(Game game, string fontName, string folder = "Fonts/")
        {
            if (ActiveGame == null)
                ActiveGame = game;

            if (LoadedFonts.ContainsKey(fontName))
                return LoadedFonts[fontName];

            string atlasPath = $"{folder}{fontName}.png";
            string schemaPath = $"{folder}{fontName}.json";

            if (!File.Exists(atlasPath))
                throw new FileNotFoundException("Atlas texture doesn't exist!");

            if (!File.Exists(schemaPath))
                throw new FileNotFoundException("Json schema doesn't exist!");

            string jsonSchema = File.ReadAllText(schemaPath);
            ScalableFont font = JsonConvert.DeserializeObject<ScalableFont>(jsonSchema);

            font.atlasTexture = Texture2D.FromFile(ActiveGame.GraphicsDevice, atlasPath);
            LoadedFonts[fontName] = font;

            font.FontName = fontName;
            font.CacheGlyphs();

            return font;
        }

        protected void CacheGlyphs()
        {
            if (glyphs == null)
                return;

            foreach (var glyph in glyphs)
            {
                if (glyph.atlasBounds == null || glyph.planeBounds == null)
                    continue;

                glyph.CalculateLayout(atlas);
                GlyphTable.Add(glyph.unicode, glyph);
            }

            glyphs = null;
        }


        // MSDF font drawing is deferred!
        // Make sure to call "MSDFFont.RenderAll()" to render things in the local context!
        public class MSDFDrawSettings
        {
            public float pixelSize = 64;
            public Matrix matrix = Matrix.Identity;
            public Color color = Color.White;

            public float horizontalAlign = 0F;
            public float verticalAlign = 0F;
            public bool rtl = false;
        }

        public delegate void MSDFFontDrawCall(SpriteBatch batch, Matrix? matrix);
        protected Queue<MSDFFontDrawCall> drawCalls = new Queue<MSDFFontDrawCall>();

        // Measures the imaginary box around this string
        public Vector2 MeasureString(string text, float pixelSize)
        {
            if (string.IsNullOrEmpty(text))
                return Vector2.Zero;

            float pixOffset = pixelSize / atlas.size;

            float lineHeight = metrics.lineHeight * pixOffset / 2;

            float actualHeight = atlas.size * lineHeight;

            Vector2 scale = Vector2.Zero;
            scale.Y += actualHeight;

            float lineLength = 0;

            int l = 0;
            foreach (char c in text)
            {
                if (c == '\n')
                {
                    scale.Y += actualHeight;
                    lineLength = 0;
                    l++;
                    continue;
                }

                if (c == ' ')
                {
                    scale.X += whiteSpaceWidth;
                    l++;
                    continue;
                }

                if (GlyphTable.TryGetValue(c, out MSDFGlyph glyph))
                {
                    float size = glyph.advance;

                    // HACK: Apparently this fixes alignment in some cases?
                    if (l == text.Length - 1)
                        size += whiteSpaceWidth;

                    lineLength += size * pixelSize;
                    scale.X = MathF.Max(lineLength, scale.X);
                }

                l++;
            }

            return scale;
        }

        public enum HAlignment
        {
            Left, Center, Right
        }

        public enum VAlignment
        {
            Top, Middle, Bottom
        }

        public static float GetHAlignWeight(HAlignment alignment)
        {
            switch (alignment)
            {
                default:
                    return 0;

                case HAlignment.Center:
                    return 0.5F;

                case HAlignment.Right:
                    return 1F;
            }
        }

        public static float GetVAlignWeight(VAlignment alignment)
        {
            switch (alignment)
            {
                default:
                    return 0;

                case VAlignment.Middle:
                    return 0.5F;

                case VAlignment.Bottom:
                    return 1F;
            }
        }

        public void Draw(string text, Matrix matrix, float halign = 0F, float valign = 0F, float pixelSize = 64, Color color = default, bool rtl = false)
        {
            MSDFDrawSettings settings = new MSDFDrawSettings()
            {
                pixelSize = pixelSize,
                matrix = matrix,
                color = color,
                horizontalAlign = halign,
                verticalAlign = valign,
                rtl = rtl
            };

            Draw(text, settings);
        }

        public void Draw(string text, Vector3 position, Quaternion rotation, Vector2 scale, float halign = 0F, float valign = 0F, float pixelSize = 64, Color color = default, bool rtl = false)
        {
            Matrix mat = Matrix.CreateFromQuaternion(rotation)
                    * Matrix.CreateScale(new Vector3(scale, 1))
                    * Matrix.CreateTranslation(position);

            Draw(text, mat, halign, valign, pixelSize, color, rtl);
        }

        public void Draw(string text, Vector2 position, float depth, float angle, Vector2 scale, float halign = 0F, float valign = 0F, float pixelSize = 64, Color color = default, bool rtl = false)
            => Draw(text, new Vector3(position, depth - 1F), Quaternion.CreateFromAxisAngle(Vector3.Forward, angle), scale, halign, valign, pixelSize, color);

        public void Draw(string text, Vector2 position, float depth, float angle, Vector2 scale, HAlignment halign = HAlignment.Left, VAlignment valign = VAlignment.Top, float pixelSize = 64, Color color = default, bool rtl = false)
            => Draw(text, position, depth, angle, scale, GetHAlignWeight(halign), GetVAlignWeight(valign), pixelSize, color, rtl);

        public void Draw(string text, Vector2 position, float depth, float halign = 0F, float valign = 0F, float pixelSize = 64, Color color = default, bool rtl = false)
            => Draw(text, position, depth, 0F, Vector2.One, halign, valign, pixelSize, color, rtl);

        public void Draw(string text, Vector2 position, float halign = 0F, float valign = 0F, float pixelSize = 64, Color color = default, bool rtl = false)
            => Draw(text, position, 0F, 0F, Vector2.One, halign, valign, pixelSize, color, rtl);

        public void Draw(string text, MSDFDrawSettings settings)
        {
            drawCalls.Enqueue((batch, matrix) =>
            {
                string capture = text;
                MSDFDrawSettings drawSettings = settings;
                ScalableFont fontCapture = this;

                string technique = drawSettings.pixelSize > 12 ? "LargeText" : "SmallText";

                // TODO: Does this severely effect performance?
                MSDFShader.Parameters["WorldViewProjection"].SetValue(drawSettings.matrix * (matrix ?? Matrix.Identity));
                MSDFShader.Parameters["ForegroundColor"].SetValue(drawSettings.color.ToVector4());
                MSDFShader.CurrentTechnique = MSDFShader.Techniques[technique];

                Vector2 wholeScale = MeasureString(capture, drawSettings.pixelSize);
                Vector2 offset = wholeScale * new Vector2(0, -drawSettings.verticalAlign);

                foreach (string token in capture.Split('\n'))
                {
                    string line = token.TrimEnd(' ', '\n', '\r');

                    Vector2 position = Vector2.Zero;
                    Vector2 localScale = MeasureString(line, drawSettings.pixelSize);

                    float halign = drawSettings.horizontalAlign;

                    position -= localScale * new Vector2(halign, 0);
                    position -= offset;

                    foreach (char c in line)
                    {
                        if (char.IsWhiteSpace(c))
                        {
                            position.X += whiteSpaceWidth * drawSettings.pixelSize;
                            continue;
                        }

                        if (fontCapture.GlyphTable.TryGetValue(c, out MSDFGlyph glyph))
                        {
                            if (glyph == null)
                                continue;

                            Vector4 glyphPack = new Vector4(
                                glyph.atlasBounds.left,
                                glyph.atlasBounds.bottom,
                                glyph.atlasBounds.right,
                                glyph.atlasBounds.top
                            );

                            MSDFShader.Parameters["Offset"].SetValue(position);
                            MSDFShader.Parameters["GlyphBB"].SetValue(glyphPack);
                            MSDFShader.CurrentTechnique.Passes[0].Apply();

                            // TODO: Better drawing parameters?
                            var plane = glyph.planeBounds;
                            Quad.Render(
                                ActiveGame.GraphicsDevice,
                                new Vector2(plane.left, plane.top) * drawSettings.pixelSize,
                                new Vector2(plane.right, plane.bottom) * drawSettings.pixelSize
                            );

                            position.X += glyph.advance * drawSettings.pixelSize * (drawSettings.rtl ? -1F : 1F);
                        }
                    }

                    offset.Y -= drawSettings.pixelSize;
                }
            });
        }

        public static void RenderAll(SpriteBatch batch, Matrix? matrix = null)
        {
            if (MSDFShader == null)
                MSDFShader = ActiveGame.Content.Load<Effect>("FieldFontEffect");

            foreach (var font in LoadedFonts.Values)
                font.Render(batch, matrix);
        }

        public void Render(SpriteBatch batch, Matrix? matrix = null)
        {
            MSDFShader.Parameters["TextureSize"].SetValue(atlasTexture.GetSize());
            MSDFShader.Parameters["PxRange"].SetValue(atlas.distanceRange);
            MSDFShader.Parameters["GlyphTexture"].SetValue(atlasTexture);
            MSDFShader.CurrentTechnique.Passes[0].Apply();

            var previousState = ActiveGame.GraphicsDevice.RasterizerState;
            ActiveGame.GraphicsDevice.RasterizerState = RasterizerState;

            using (FontDrawingLayer.BeginScoped(batch, matrix))
            {
                while (drawCalls.Count > 0)
                {
                    var call = drawCalls.Dequeue();
                    call?.Invoke(batch, matrix);
                }
            }

            ActiveGame.GraphicsDevice.RasterizerState = previousState;
        }
    }
}
