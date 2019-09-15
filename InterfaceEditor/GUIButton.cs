﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.ComponentModel;
using System.Drawing.Design;

namespace InterfaceEditor
{
	class GUIButton : GUIPanel
	{
		public bool IsDisabled { get; set; }

		protected string _PressedImageName;
		protected Image _PressedImageContent;
		[Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
		public string PressedImage
		{
			get
			{
				return _PressedImageName;
			}
			set
			{
				_PressedImageName = value;
				SetImage(_PressedImageName, ref _PressedImageContent);
			}
		}

		protected ImageLayout _PressedImageLayout;
		public ImageLayout PressedImageLayout
		{
			get
			{
				return _PressedImageLayout;
			}
			set
			{
				_PressedImageLayout = value;
			}
		}

		protected string _HoverImageName;
		protected Image _HoverImageContent;
		[Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
		public string HoverImage
		{
			get
			{
				return _HoverImageName;
			}
			set
			{
				_HoverImageName = value;
				SetImage(_HoverImageName, ref _HoverImageContent);
			}
		}

		protected ImageLayout _HoverImageLayout;
		public ImageLayout HoverImageLayout
		{
			get
			{
				return _HoverImageLayout;
			}
			set
			{
				_HoverImageLayout = value;
			}
		}

		protected string _DisabledImageName;
		protected Image _DisabledImageContent;
		[Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
		public string DisabledImage
		{
			get
			{
				return _DisabledImageName;
			}
			set
			{
				_DisabledImageName = value;
				SetImage(_DisabledImageName, ref _DisabledImageContent);
			}
		}

		protected ImageLayout _DisabledImageLayout;
		public ImageLayout DisabledImageLayout
		{
			get
			{
				return _DisabledImageLayout;
			}
			set
			{
				_DisabledImageLayout = value;
			}
		}

		public GUIButton(GUIObject parent)
			: base(parent)
		{
		}

		override public bool IsAutoSize()
		{
			return _PressedImageLayout == ImageLayout.None || _HoverImageLayout == ImageLayout.None || _DisabledImageLayout == ImageLayout.None || base.IsAutoSize();
		}
		override public void DoAutoSize()
		{
			base.DoAutoSize();

			if (_PressedImageContent != null)
				Size = _PressedImageContent.Size;
			if (_HoverImageContent != null)
				Size = _HoverImageContent.Size;
			if (_DisabledImageContent != null)
				Size = _DisabledImageContent.Size;
		}

		public override void DrawPass1(Graphics g)
		{
			if (_PressedImageContent != null)
				g.DrawImage(_PressedImageContent, new Rectangle(AbsolutePosition, Size));

			base.DrawPass1(g);
		}
	}
}
