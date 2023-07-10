using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace zCubed.MSDF
{
    public class DrawingLayer
    {
        public SpriteSortMode sortMode = SpriteSortMode.BackToFront;
        public BlendState blendState = BlendState.AlphaBlend;
        public SamplerState samplerState = SamplerState.LinearWrap;
        public Matrix? matrix = null;
        public Effect effect = null;

        public SpriteBatch batch;

        public static DrawingLayer ActiveLayer;

        public DrawingLayer() { }

        public DrawingLayer(SpriteSortMode sortMode, BlendState blendState, SamplerState samplerState, Matrix? matrix, Effect effect)
        {
            this.sortMode = sortMode;
            this.blendState = blendState;
            this.samplerState = samplerState;
            this.matrix = matrix;
            this.effect = effect;
        }

        public void Begin(SpriteBatch batch)
        {
            this.batch = batch;
            ActiveLayer = this;

            batch.Begin(sortMode, blendState, samplerState, DepthStencilState.Default, null, effect, matrix);
        }

        public void End(SpriteBatch batch)
        {
            this.batch = null;
            ActiveLayer = null;

            batch.End();
        }

        public ScopedLayer BeginScoped(SpriteBatch batch, Matrix? matrix = null, Effect effect = null)
        {
            return new ScopedLayer(batch, this, matrix, effect);
        }
    }

    public class ScopedLayer : IDisposable
    {
        protected SpriteBatch batch;
        protected DrawingLayer layer;

        public ScopedLayer(SpriteBatch batch, DrawingLayer layer, Matrix? matrix = null, Effect effect = null)
        {
            layer.matrix = matrix;
            layer.effect = effect;
            layer.Begin(batch);

            this.layer = layer;
            this.batch = batch;
        }

        public ScopedLayer(SpriteBatch batch, DrawingLayer layer) : this(batch, layer, null) { }

        public void Dispose()
        {
            layer.End(batch);
        }
    }
}
