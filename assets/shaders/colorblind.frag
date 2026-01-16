uniform sampler2D texture;
uniform int mode; // 0=Normal, 1=Protanopia, 2=Deuteranopia, 3=Tritanopia, 4=HighContrast

void main()
{
    vec4 color = texture2D(texture, gl_TexCoord[0].xy) * gl_Color;

    if (mode == 1) {
        // Protanopia (rouge-aveugle) - Simulation basée sur les matrices de Brettel
        mat3 protanopia = mat3(
            0.567, 0.433, 0.000,
            0.558, 0.442, 0.000,
            0.000, 0.242, 0.758
        );
        color.rgb = protanopia * color.rgb;
    }
    else if (mode == 2) {
        // Deuteranopia (vert-aveugle)
        mat3 deuteranopia = mat3(
            0.625, 0.375, 0.000,
            0.700, 0.300, 0.000,
            0.000, 0.300, 0.700
        );
        color.rgb = deuteranopia * color.rgb;
    }
    else if (mode == 3) {
        // Tritanopia (bleu-aveugle)
        mat3 tritanopia = mat3(
            0.950, 0.050, 0.000,
            0.000, 0.433, 0.567,
            0.000, 0.475, 0.525
        );
        color.rgb = tritanopia * color.rgb;
    }
    else if (mode == 4) {
        // High Contrast - Augmente la saturation et le contraste
        float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        color.rgb = mix(vec3(gray), color.rgb, 1.5); // Augmente saturation
        color.rgb = (color.rgb - 0.5) * 1.3 + 0.5; // Augmente contraste
        color.rgb = clamp(color.rgb, 0.0, 1.0);
    }

    gl_FragColor = color;
}
