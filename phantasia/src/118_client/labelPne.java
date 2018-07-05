import java.awt.*;

public class labelPne extends Canvas implements constants {

    pClient parent = null;
    int imageNumber = -1;
    Rectangle theBounds = null;
    int canvasWidth = 0;

    public labelPne(pClient c) {

	super();
	parent = c;
	setBackground(backgroundColor);
    }

    public void paint(Graphics g) {

	theBounds = getBounds();
	canvasWidth = theBounds.width - 1;

	g.setColor(highlightColor);
	g.drawLine(0, 4, canvasWidth, 4);
	g.drawLine(0, 2, 0, 6);
	g.drawLine(canvasWidth, 2, canvasWidth, 6);

	if (imageNumber != -1 && parent.theImages[imageNumber] != null) {
	    g.drawImage(parent.theImages[imageNumber], (canvasWidth - 9) / 2,
		    0, this);
	}
    }

    public void setImage(int newImage) {

	imageNumber = newImage;
	repaint();
    }
}

